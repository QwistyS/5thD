#include <chrono>
#include <memory>
#include <thread>
#include "signal.h"

#include "5thdlogger.h"
#include "5thderrors.h"
#include "5thderror_handler.h"
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
    NetworkError error;

    int duration_ms = 100;

    std::unique_ptr<ZMQBus> bus = std::make_unique<ZMQBus>(IPC_ROUTER_ADDR, 0, &error);

    while (termination_requested) {
        bus->run();
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    }

    DEBUG("Clearing router...");
    return 0;
}
