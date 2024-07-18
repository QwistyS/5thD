#include <sys/param.h>
#include <chrono>
#include <cstring>
#include <memory>
#include "5thdipcmsg.h"
#include "izmq.h"
#include "keys_db.h"
#include "module.h"
#include "zmq.h"

#ifdef _WIN32
#    include "proc_win.h"
#    include "win_implementation.h"
#elif defined(__linux__)
#    include "proc_unix.h"
#elif defined(__APPLE__)
#else
#    error "Unknown operating system"
#endif

#include "5thdipc_client.h"
#include "5thdipcmsg.h"
#include "5thdtracy.h"
#include "module.h"
#include "transmitter.h"

void foo() {
    TELEMETRY("foo")
    DEBUG("Foo is called");
}

void bar() {
    TELEMETRY("foo")
    DEBUG("Bar is called");
}
int main() {
    module_init_t config;
    config.keys_info.is_ready = false;
    config.keys_info.key_type = KeyType::CURVE25519;
    config.client_id = Clients::MANAGER;
    std::string ipcpub_key;
    ipc_msg_t ipc_msg_manager;

    module_init(&config);

    // // Set up signal handler
    // signal(SIGINT, signal_handler);
    // signal(SIGTERM, signal_handler);

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
        *ipcsock, CLIENTS_IDS[static_cast<int>(Clients::MANAGER)]);

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
    // test for tracy dinamic stuff with no branching
    TracyWrapper& profiler = TracyWrapper::getInstance();
    // Thread to control profiling
    std::atomic<bool> runControlThread(true);
    // std::thread controlThread([&profiler, &runControlThread]() {
    //     std::cout << "Press 's' to start profiling, 'e' to end profiling, 'q' to quit:" << std::endl;
    //     char command;
    //     while (runControlThread) {
    //         std::cin >> command;
    //         if (command == 's') {
    //             profiler.setProfilingEnabled(true);
    //             std::cout << "Profiling started." << std::endl;
    //         } else if (command == 'e') {
    //             profiler.setProfilingEnabled(false);
    //             std::cout << "Profiling ended." << std::endl;
    //         } else if (command == 'q') {
    //             runControlThread = false;
    //             break;
    //         }
    //     }
    // });

    // Main workload
    while (runControlThread) {
        TELEMETRY("Main Workload")
        foo();
        bar();

        ipc_msg(&ipc_msg_manager, Clients::MANAGER, Clients::PEER);
        ipc_client->send(&ipc_msg_manager);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up control thread
    // if (controlThread.joinable()) {
    //     controlThread.join();
    // }
    ipc_trans->disconnect(IPC_ENDPOINT, 0);
    ipcsock.reset();
    ctx->close();

    return 0;
}
