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

VoidResult print_schema(DatabaseAccess& db) {
    std::string sql = "SELECT sql FROM sqlite_master WHERE type='table';";
    sdbret_t result;
    auto key_stmt = db.prepare(sql);
    if (key_stmt.is_err())
        return Err(ErrorCode::FAIL_GET_KEY, "Failed to prepare key query");

    auto key_stmt_ptr = key_stmt.value();
    auto query_result = db.query(key_stmt_ptr, result);
    if (query_result.is_err()) {
        ERROR("Failed to query schema: {}", query_result.error().message());
        return Err(ErrorCode::DB_ERROR, "Failed to query schema");
    }
    for (const auto& row : result.rows) {
        if (!row.columns.empty()) {
            DEBUG("Table schema: {}", std::string(row.columns[0].second.begin(), row.columns[0].second.end()));
        }
    }
    return Ok();
}

int main() {
    Log::init();
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (sodium_init() < 0) {
        ERROR("Failed to initialize libsodium");
        std::abort();
    }
    DatabaseAccess db;
    print_schema(db);

    auto key_result = get_key(db, CLIENTS_IDS[Clients::ROUTER], KeyType::CURVE25519, "public_key");
    if (key_result.is_err()) {
        WARN("Error from db {}", key_result.error().message());
        if (key_result.error().code() == ErrorCode::KEY_NOT_FOUND) {
            char public_key[41];
            char private_key[41];
            if (generate_keys(public_key, private_key) != static_cast<int>(ErrorCode::OK)) {
                ERROR("Failed to generate keys for router");
                std::abort();
            }
            DEBUG("Key generated private {} public {}", public_key, private_key);

            // Convert char arrays to vectors of unsigned char
            std::vector<unsigned char> public_key_vec(public_key, public_key + strlen(public_key));
            std::vector<unsigned char> private_key_vec(private_key, private_key + strlen(private_key));

            // Store public key
            auto ret_public =
                store_key(db, CLIENTS_IDS[Clients::ROUTER], KeyType::CURVE25519, "public_key", public_key_vec);
            if (ret_public.is_err()) {
                ERROR("Failed to store public key: {}", ret_public.error().message());
                std::abort();
            }
            DEBUG("Keys public stored");

            // Store private key
            auto ret_private =
                store_key(db, CLIENTS_IDS[Clients::ROUTER], KeyType::CURVE25519, "private_key", private_key_vec);
            if (ret_private.is_err()) {
                ERROR("Failed to store private key: {}", ret_private.error().message());
                std::abort();
            }

            DEBUG("Generated and stored new keys for router");
        } else {
            // Handle other errors
            ERROR("Error while checking for existing key: {}", key_result.error().message());
            std::abort();
        }
    } else {
        DEBUG("Existing keys found for router");
    }

    std::unique_ptr<ZMQWContext> ctx = std::make_unique<ZMQWContext>();
    std::unique_ptr<ZMQWSocket> socket = std::make_unique<ZMQWSocket>(ctx.get(), ZMQ_ROUTER);

    ZMQReceiver recv("", 0, ctx.get(), socket.get());

    recv.set_endpoint(IPC_ENDPOINT);
    std::unique_ptr<ZMQBus> bus = std::make_unique<ZMQBus>(&recv);

    bus->run();

    socket.reset();
    ctx.reset();
    DEBUG("Clearing router...");
    return 0;
}
