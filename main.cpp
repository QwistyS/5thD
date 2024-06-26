#include <argumentum/argparse.h>

#include "5thdlogger.h"
#include "peer.h"
#include "5thdlru_cache.h"


int main(int argc, char* argv[]) {
    char command[10];
    void* data = nullptr;
    LRU_Cache<std::string, Connection> lruc(10);

    Log::init();
    int port = is_port_available(START_PORT);
    if (port == -1)
        return port;
    Peer peer(port);
    // peer.listen();
    peer.connect("127.0.0.1", START_PORT);

    if (fgets(command, sizeof(command), stdin) == NULL) {
        perror("Error reading input");
        return 1;
    }
    return 0;
}
