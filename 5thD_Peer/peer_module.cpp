#include <signal.h>
#include <zmq.h>
#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "5thdsql.h"
#include "keys_db.h"
#include "module.h"
#include "net_helpers.h"
#include "peer.h"
#include "transmitter.h"

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
    memset(msg, 0, sizeof(ipc_msg_t));
    msg->src_id = src;
    msg->dist_id = dist;
    msg->timestamp = time(nullptr);
}

int main() {
    module_init_t config;
    memset(&config, 0, sizeof(module_init_t));
    config.keys_info.is_ready = false;
    config.keys_info.key_type = KeyType::CURVE25519;
    config.client_id = Clients::PEER;

    module_init(&config);

    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (sodium_init() < 0) {
        ERROR("Failed to initialize libsodium");
        return 1;
    }

    std::string ipcpub_key;

    DatabaseAccess db;
    auto ipcrout_pub_key =
        get_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519, "public_key");

    if (ipcrout_pub_key.is_err()) {
        ERROR("IPCROUT Public key is unavailable");
        std::abort();
    }

    ipcpub_key.assign(ipcrout_pub_key.value().begin(), ipcrout_pub_key.value().end());

    // Start ipc
    auto ctx = std::make_unique<ZMQWContext>();
    auto ipcsock = std::make_unique<ZMQWSocket>(ctx.get(), ZMQ_DEALER);
    auto ipc_trans = std::make_unique<ZMQWTransmitter>(ipcsock.get(), CLIENTS_IDS[static_cast<int>(Clients::PEER)]);

    int rc = ipc_trans->set_sockopt(ZMQ_CURVE_SERVERKEY, ipcpub_key.c_str(), 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SERVERKEY: {}", zmq_strerror(errno));
    }
    rc = ipc_trans->set_sockopt(ZMQ_CURVE_PUBLICKEY, config.keys_info.curve_pub, 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_PUBLICKEY: {}", zmq_strerror(errno));
    }
    rc = ipc_trans->set_sockopt(ZMQ_CURVE_SECRETKEY, config.keys_info.curve_prv, 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SECRETKEY: {}", zmq_strerror(errno));
    }

    config.keys_info.deinit();
    ipcpub_key.clear();

    int timeout_ms = 5000;  // 1 seconds
    ipc_trans->set_sockopt(ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
    ipc_trans->set_sockopt(ZMQ_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));

    auto ipc_client = std::make_unique<IpcClient>(ipc_trans.get());
    // Register self id.
    ipc_msg(&ipc_peer_msg, Clients::PEER, Clients::ROUTER);

    print_ipc_msg(&ipc_peer_msg);

    while (termination_requested) {
        DEBUG("Sending data");
        ipc_client->send(&ipc_peer_msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}