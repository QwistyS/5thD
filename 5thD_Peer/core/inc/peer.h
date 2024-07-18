#ifndef PEER_H
#define PEER_H

#include "5thderror_handler.h"
#include "receiver.h"
#include "transmitter.h"

#define START_PORT 7099
#define CACHE_SIZE 100

template <ITransmiterConcept Transmitter, IReceiverConcept Reciever>
class Peer final {
public:
    Peer(Transmitter& trancmitter, Reciever& receiver) : _error(_drp), _transmitter(trancmitter), _receiver(receiver) {
        _init();
    }
    ~Peer();

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    Transmitter& _transmitter;
    Reciever& _receiver;
    void _init();
};

#endif  // PEER_H