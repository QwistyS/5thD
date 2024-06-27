#include <argumentum/argparse.h>

#include "5thdlogger.h"
#include "peer.h"
#include "net_helpers.h"
#include "5thdlru_cache.h"


int main(int argc, char* argv[]) {
    Log::init();

    char command[10];
    void* data = nullptr;
    int port = is_port_available(START_PORT);
    std::string internal_addr = get_local_ip();

    if(internal_addr.empty()) {
        ERROR("Fail to determinate local ip address");
        return 0;
    }

    std::string iface = get_iface_name(internal_addr);
    if(iface.empty()) {
        ERROR("Fail tp determinate iface");
        return 0;
    }

    DEBUG("Internal ip is {}", internal_addr);

    if (port == -1)
        return port;
    
    set_port_forward(port, internal_addr, port, iface);
    Peer peer(port);
    peer.listen();
    peer.connect("127.0.0.1", START_PORT);

    if (fgets(command, sizeof(command), stdin) == NULL) {
        perror("Error reading input");
        return 1;
    }
    return 0;
}
