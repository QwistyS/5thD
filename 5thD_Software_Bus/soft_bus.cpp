#include <zmq.h>
#include <chrono>
#include <memory>
#include <thread>
#include "signal.h"

#include "5thdsql.h"
#include "keys_db.h"
#include "software_bus.h"

// Global flag to indicate if termination signal received
volatile sig_atomic_t termination_requested = 1;

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        DEBUG("Termination signal received. Cleaning up...");
        termination_requested = 0;
    }
}

int main() {
    Log::init();
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::string router_pub_key;
    std::string router_prv_key;

    if (sodium_init() < 0) {
        ERROR("Failed to initialize libsodium");
        return 1;
    }

    DatabaseAccess db;

    // Scope to clear RAM
    {
        // Try to get existing keys
        DEBUG("Attempting to retrieve existing keys");
        auto public_key_result =
            get_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519, "public_key");
        auto private_key_result =
            get_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519, "private_key");

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
            auto store_public = store_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519,
                                          "public_key", public_key);
            if (store_public.is_err()) {
                ERROR("Failed to store public key: {}", store_public.error().message());
                db.exec("ROLLBACK");  // Rollback the transaction if storing public key fails
                return 1;
            }
            DEBUG("Public key stored successfully");

            DEBUG("Storing private key");
            auto store_private = store_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519,
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

            router_pub_key.assign(public_key.begin(), public_key.end());
            router_prv_key.assign(private_key.begin(), private_key.end());

        } else {
            DEBUG("Existing keys found");
            router_pub_key.assign(public_key_result.value().begin(), public_key_result.value().end());
            router_prv_key.assign(private_key_result.value().begin(), private_key_result.value().end());
            QWISTYS_TODO_MSG("Clear the keys from RAM");
        }
    }

    std::unique_ptr<ZMQWContext> ctx = std::make_unique<ZMQWContext>();
    std::unique_ptr<ZMQWSocket> socket = std::make_unique<ZMQWSocket>(ctx.get(), ZMQ_ROUTER);

    ZMQReceiver recv(CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], 0, ctx.get(), socket.get());

    recv.set_endpoint(IPC_ENDPOINT);
    std::unique_ptr<ZMQBus> bus = std::make_unique<ZMQBus>(&recv);

    bus->set_security(router_pub_key.c_str(), router_prv_key.c_str());
    
    router_pub_key.clear();

    bus->run();

    DEBUG("Clearing router...");
    return 0;
}
