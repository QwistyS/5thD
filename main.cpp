#include "5thdlogger.h"
#include "peer.h"

int main(int argc, char* argv[]) {
    Log::init();
    Peer peer(123123);
    peer.connect("someaddress", 929);
    return 0;
}
