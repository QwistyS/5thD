#include <signal.h>
#include <zmq.h>
#include <chrono>
#include <cstring>
#include <thread>

#include "5thdipc_client.h"
#include "5thdipcmsg.h"
#include "5thdsql.h"
#include "izmq.h"
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



int main() {
    module_init_t config;
    config.keys_info.is_ready = false;
    config.keys_info.key_type = KeyType::CURVE25519;
    config.client_id = Clients::PEER;

    module_init(&config);

    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

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
    auto ipcsock = std::make_unique<ZMQWSocket<ZMQWContext>>(*ctx, ZMQ_DEALER);
    auto ipc_trans = std::make_unique<ZMQWTransmitter<ZMQWSocket<ZMQWContext>>>(
        *ipcsock, CLIENTS_IDS[static_cast<int>(Clients::PEER)]);

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

    auto ipc_client = std::make_unique<IpcClient<ZMQWTransmitter<ZMQWSocket<ZMQWContext>>>>(*ipc_trans);
    // Register self id.

    print_ipc_msg(&ipc_peer_msg);

    while (termination_requested) {
        DEBUG("Sending data");
        ipc_msg(&ipc_peer_msg, Clients::PEER, Clients::MANAGER);
        ipc_client->send(&ipc_peer_msg);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ipc_trans.reset();
    ipcsock.reset();
    ctx->close();
    ctx.reset();
    return 0;
}
