#include <signal.h>
#include <zmq.h>
#include <chrono>
#include <memory>
#include <thread>

#include "5thderror_handler.h"
#include "net_helpers.h"
#include "peer.h"

#include "5thdsql.h"
#include "keys_db.h"

// Global flag to indicate if termination signal received
volatile sig_atomic_t termination_requested = 1;

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        DEBUG("Termination signal received. Cleaning up..");
        termination_requested = 0;
    }
}

static ipc_msg_t ipc_peer_msg;

void ipc_msg(ipc_msg_t* msg, int src, int dist) {
    msg->src_id = src;
    msg->dist_id = dist;
    msg->timestamp = time(NULL);
}

int main() {
    Log::init();
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (sodium_init() < 0) {
        ERROR("Failed to initialize libsodium");
        return 1;
    }

    std::string peer_pub_key;
    std::string peer_prv_key;
    std::string ipcpub_key;

    DatabaseAccess db;
    auto ipcrout_pub_key =
        get_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519, "public_key");

    if (ipcrout_pub_key.is_err()) {
        ERROR("IPCROUT Public key is unavailable");
        std::abort();
    }
    ipcpub_key.assign(ipcrout_pub_key.value().begin(), ipcrout_pub_key.value().end());

    // Scope to clear RAM
    {
        // Try to get existing keys
        DEBUG("Attempting to retrieve existing keys");
        auto public_key_result =
            get_key(db, CLIENTS_IDS[static_cast<int>(Clients::PEER)], KeyType::CURVE25519, "public_key");
        auto private_key_result =
            get_key(db, CLIENTS_IDS[static_cast<int>(Clients::PEER)], KeyType::CURVE25519, "private_key");

        if (public_key_result.is_err() || private_key_result.is_err()) {
            DEBUG("Keys not found, generating new ones");
            char public_key_str[41];
            char private_key_str[41];
            if (generate_keys(public_key_str, private_key_str) != static_cast<int>(ErrorCode::OK)) {
                ERROR("Failed to generate keys for router");
                return 1;
            }
            DEBUG("Keys generated: public {}, private {}", public_key_str, private_key_str);

            std::vector<unsigned char> public_key(public_key_str, public_key_str + strlen(public_key_str));
            std::vector<unsigned char> private_key(private_key_str, private_key_str + strlen(private_key_str));

            auto begin_result = db.begin_transaction();
            if (begin_result.is_err()) {
                ERROR("Failed to begin transaction: {}", begin_result.error().message());
                return 1;
            }

            DEBUG("Storing public key");
            auto store_public = store_key(db, CLIENTS_IDS[static_cast<int>(Clients::PEER)], KeyType::CURVE25519,
                                          "public_key", public_key);
            if (store_public.is_err()) {
                ERROR("Failed to store public key: {}", store_public.error().message());
                db.exec("ROLLBACK");  // Rollback the transaction if storing public key fails
                return 1;
            }
            DEBUG("Public key stored successfully");

            DEBUG("Storing private key");
            auto store_private = store_key(db, CLIENTS_IDS[static_cast<int>(Clients::PEER)], KeyType::CURVE25519,
                                           "private_key", private_key);
            if (store_private.is_err()) {
                ERROR("Failed to store private key: {}", store_private.error().message());
                db.exec("ROLLBACK");  // Rollback the transaction if storing private key fails
                return 1;
            }
            DEBUG("Private key stored successfully");

            auto end_result = db.end_transaction();
            if (end_result.is_err()) {
                ERROR("Failed to commit transaction: {}", end_result.error().message());
                return 1;
            }
            peer_pub_key.assign(public_key.begin(), public_key.end());
            peer_prv_key.assign(private_key.begin(), private_key.end());

        } else {
            DEBUG("Existing keys found");

            peer_pub_key.assign(public_key_result.value().begin(), public_key_result.value().end());
            peer_prv_key.assign(private_key_result.value().begin(), private_key_result.value().end());
            QWISTYS_TODO_MSG("Clear the keys from RAM");
        }
    }

    // Start ipc
    ZMQWContext ipc_ctx;
    ZMQWSocket ipc_sock(&ipc_ctx, ZMQ_DEALER);
    ZMQTransmitter ipc_transmitter(&ipc_ctx, &ipc_sock, CLIENTS_IDS[static_cast<int>(Clients::PEER)]);
    DEBUG("Server public key length: {}", ipcpub_key.length());
    DEBUG("Client public key length: {}", peer_pub_key.length());
    DEBUG("Client private key length: {}", peer_prv_key.length());

    int rc = ipc_transmitter.set_sockopt(ZMQ_CURVE_SERVERKEY, ipcpub_key.c_str(), 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SERVERKEY: {}", zmq_strerror(errno));
    }
    rc = ipc_transmitter.set_sockopt(ZMQ_CURVE_PUBLICKEY, peer_pub_key.c_str(), 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_PUBLICKEY: {}", zmq_strerror(errno));
    }
    rc = ipc_transmitter.set_sockopt(ZMQ_CURVE_SECRETKEY, peer_prv_key.c_str(), 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SECRETKEY: {}", zmq_strerror(errno));
    }

    int timeout_ms = 5000;  // 1 seconds
    ipc_transmitter.set_sockopt(ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
    ipc_transmitter.set_sockopt(ZMQ_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));

    IpcClient ipc_client(&ipc_transmitter);

    // Register self id.
    ipc_msg(&ipc_peer_msg, Clients::PEER, Clients::PEER);
    while (termination_requested) {
        DEBUG("Sending data");
        ipc_client.send(&ipc_peer_msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}