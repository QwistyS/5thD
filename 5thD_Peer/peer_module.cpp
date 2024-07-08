#include <signal.h>
#include <zmq.h>
#include <chrono>
#include <memory>
#include <thread>

#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "net_helpers.h"
#include "peer.h"

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

    // Start ipc
    ZMQWContext ipc_ctx;
    ZMQWSocket ipc_sock(&ipc_ctx, ZMQ_DEALER);
    ZMQTransmitter ipc_transmitter(&ipc_ctx, &ipc_sock, CLIENTS_IDS[Clients::PEER]);
    IpcClient ipc_client(&ipc_transmitter);

    // Register self id.
    ipc_msg(&ipc_peer_msg, Clients::PEER, Clients::PEER);
    ipc_client.send(&ipc_peer_msg);


    return 0;
}