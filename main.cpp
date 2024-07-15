#include <cstring>
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "5thdlogger.h"
#include "keys_db.h"

#ifdef _WIN32
#include "proc_win.h"
#include "win_implementation.h"
#elif defined(__linux__)
#include "proc_unix.h"
#elif defined(__APPLE__)
#else
#error "Unknown operating system"
#endif

#include "module.h"


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


    DEBUG(BIN_PATH);
    return 0;
}
