#include "5thdlogger.h"
#include "peer.h"

int main(int argc, char* argv[]) {
    void *data = nullptr;
    Log::init();
    int port = is_port_available(START_PORT);
    if (port == -1) return port;
    Peer peer(port);
    peer.listen();
    peer.connect("127.0.0.1", 5555);
    peer.send(data, 0);
    return 0;
}
