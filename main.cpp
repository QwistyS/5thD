#include <sys/param.h>
#include <chrono>
#include <cstring>
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "5thdlogger.h"
#include "keys_db.h"

#ifdef _WIN32
#    include "proc_win.h"
#    include "win_implementation.h"
#elif defined(__linux__)
#    include "proc_unix.h"
#elif defined(__APPLE__)
#else
#    error "Unknown operating system"
#endif

#include "5thdtracy.h"
#include "module.h"

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
    config.client_id = Clients::MANAGER;
    config.keys_info.is_ready = false;
    config.keys_info.key_type = KeyType::CURVE25519;

    module_init(&config);

    DatabaseAccess db;
    if (auto ipcrout_pub_key =
            get_key(db, CLIENTS_IDS[static_cast<int>(Clients::ROUTER)], KeyType::CURVE25519, "public_key");
        ipcrout_pub_key.is_err()) {
        ERROR("IPCROUT Public key is unavailable");
        std::abort();
    }

    TracyWrapper& profiler = TracyWrapper::getInstance();

    // Thread to control profiling
    std::atomic<bool> runControlThread(true);
    std::thread controlThread([&profiler, &runControlThread]() {
        std::cout << "Press 's' to start profiling, 'e' to end profiling, 'q' to quit:" << std::endl;
        char command;
        while (runControlThread) {
            std::cin >> command;
            if (command == 's') {
                profiler.setProfilingEnabled(true);
                std::cout << "Profiling started." << std::endl;
            } else if (command == 'e') {
                profiler.setProfilingEnabled(false);
                std::cout << "Profiling ended." << std::endl;
            } else if (command == 'q') {
                runControlThread = false;
                break;
            }
        }
    });

    // Main workload
    while (runControlThread) {
        TELEMETRY("Main Workload")
        foo();
        bar();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    // Clean up control thread
    if (controlThread.joinable()) {
        controlThread.join();
    }

    return 0;
}
