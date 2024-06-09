#include "logger.h"
#include "peer.h"

int main(int argc, char* argv[]) {
    logger5thd::init();
    Peer peer(123123);
    peer.connect("someaddress", 929);
    return 0;
}
