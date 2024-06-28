#include "5thdlogger.h"
#include "peer.h"
#include "net_helpers.h"
#include "5thdlru_cache.h"


int main(int argc, char* argv[]) {
    Log::init();
    NetworkError error;
    char command[10];
    
    int port = is_port_available(START_PORT);
    std::string internal_addr = get_local_ip();

    conn_info_t peer_info = {"someid", internal_addr, (uint16_t)port};

    if(internal_addr.empty()) {
        ERROR("Fail to determinate local ip address");
        return 0;
    }

    std::string iface = get_iface_name(internal_addr);
    if(iface.empty()) {
        ERROR("Fail tp determinate iface");
        return 0;
    }
    
    if (port == -1)
        return port;
    
    Peer peer(port, &error);
    peer.task(peer_info, Task::TASK_LISTEN);

    // set_port_forward(port, internal_addr, port, iface);
    // DEBUG("Internal ip is {}:{}", iface, internal_addr);
    // peer.connect("127.0.0.1", START_PORT);

    if (fgets(command, sizeof(command), stdin) == NULL) {
        perror("Error reading input");
        return 1;
    }

    error.dump();
    return 0;
}
