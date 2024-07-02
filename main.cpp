#include "5thdlogger.h"
#include "5thdlru_cache.h"
#include "net_helpers.h"
#include "peer.h"
#include "5thdipc_client.h"
#include "izmq.h"

int main(int argc, char* argv[]) {
    Log::init();
    NetworkError error;
    char command[10];
    ipc_msg_t m;
    std::string id = CLIENTS_IDS[Clients::MANAGER];

    memcpy(&m.category, "category", 8);
    m.dist_id = Clients::MANAGER;
    m.src_id = Clients::MANAGER;

    std::shared_ptr<ZMQWContext> ctx = std::make_shared<ZMQWContext>(&error);
    IpcClient ipcc(ctx->get_context(), id, &error);
    

    // return 0;
    // int port = is_port_available(START_PORT);
    // std::string internal_addr = get_local_ip();
    // std::string external_addr = get_external_addr();

    // if (internal_addr.empty()) {
    //     ERROR("Fail to determinate local ip address");
    //     return 0;
    // }

    // if (external_addr.empty()) {
    //     ERROR("Fail to determinate local ip address");
    //     return 0;
    // }

    // std::string iface = get_iface_name(internal_addr);
    // if (iface.empty()) {
    //     ERROR("Fail tp determinate iface");
    //     return 0;
    // }

    // if (port == -1)
    //     return port;

    // conn_info_t peer_info = {"someid", "*", (uint16_t) port};

    // Peer peer(&error);
    // peer.task(&peer_info, TASK_RECEIVER_REINIT);
    // peer.task(&peer_info, Task::TASK_LISTEN);
    // peer.task(&peer_info, TASK_PORTFORWARD);


    while (1) {
        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Error reading input");
            return 1;
        }
        ipcc.send(&m);
        error.dump();
        if (command[0] == 'q') {
            break;
        }
    }

    return 0;
}
