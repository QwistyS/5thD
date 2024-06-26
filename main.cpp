#include <argumentum/argparse.h>

#include "5thdlogger.h"
#include "peer.h"
#include "5thdlru_cache.h"


int main(int argc, char* argv[]) {
    char command[10];
    void* data = nullptr;
    Log::init();
    int port = is_port_available(START_PORT);
    DEBUG("Local ip is {}", get_local_ip());
    if (port == -1)
        return port;
    
    // set_port_forward(port, get_local_ip(), port);
    Peer peer(port);
    // peer.listen();
    // peer.connect("127.0.0.1", START_PORT);

    // if (fgets(command, sizeof(command), stdin) == NULL) {
    //     perror("Error reading input");
    //     return 1;
    // }
    return 0;
}
