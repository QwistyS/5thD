#ifndef PEER_H
#define PEER_H

#include <unordered_set>
#include <vector>
#include "5thderror_handler.h"
#include "5thdipc_client.h"
#include "receiver.h"
#include "transmitter.h"

#define START_PORT 7099
#define CACHE_SIZE 100

class Peer {
public:
    Peer(std::string& address, int port) : _addr(address), _port(port), _error(_drp) { _init(); }
    ~Peer();

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    std::string _addr;
    int _port;
    ZMQWContext _ctx;
    ITransmitter* _transmitter;
    IReceiver* _receiver;
    void _init();
};

#endif  // PEER_H