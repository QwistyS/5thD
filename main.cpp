#include "5thdlogger.h"
#include "peer.h"

int main(int argc, char* argv[]) {
    void *data = nullptr;
    Log::init();
    Peer peer(123123);
    peer.connect("127.0.0.1", 5555);
    peer.send(data, 0);
    return 0;
}
