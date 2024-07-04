#include <chrono>
#include <memory>
#include <thread>

#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "net_helpers.h"
#include "peer.h"
#include "signal.h"

// Global flag to indicate if termination signal received
volatile sig_atomic_t termination_requested = 1;

void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        printf("Termination signal received. Cleaning up...\n");
        termination_requested = 0;
    }
}

int main() {

    return 0;
}