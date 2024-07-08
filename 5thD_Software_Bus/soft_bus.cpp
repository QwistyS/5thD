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
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (sodium_init() < 0) {
        ERROR("Failed to initialize libsodium");
        std::abort();
    }
    DatabaseAccess db;

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
