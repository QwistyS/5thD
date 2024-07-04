#include <chrono>
#include <memory>
#include <thread>
#include "signal.h"
#include <zmq.h>

#include "software_bus.h"

// Global flag to indicate if termination signal received
volatile sig_atomic_t termination_requested = 1;

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        printf("Termination signal received. Cleaning up...\n");
        termination_requested = 0;
    }
}

int main() {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    Log::init();

    int duration_ms = 100;
    ZMQWContext ctx;
    ZMQWSocket socket(&ctx, ZMQ_ROUTER);
    ZMQReceiver recv(IPC_ROUTER_ADDR, IPC_ROUTER_PORT, &ctx, &socket);
    std::unique_ptr<ZMQBus> bus = std::make_unique<ZMQBus>(&recv);

    bus->run();

    DEBUG("Clearing router...");
    return 0;
}
