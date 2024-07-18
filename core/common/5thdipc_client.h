#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include <thread>
#include "5thdipcmsg.h"
#include "5thdlogger.h"
#include "transmitter.h"

template <ITransmiterConcept Transmitter>
class IpcClient {
public:
    IpcClient(Transmitter& transmitter) : _transmitter(transmitter), _error(_drp), _poll(1) { _init(); };
    ~IpcClient();
    void send(const ipc_msg_t* msg);

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    Transmitter& _transmitter;
    std::thread _worker_thread;
    int _poll;
    void _init();
};

template <ITransmiterConcept Transmitter>
IpcClient<Transmitter>::~IpcClient() {

    _poll = 0;
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }
    DEBUG("IPC Client closed");
}

template <ITransmiterConcept Transmitter>
void IpcClient<Transmitter>::send(const ipc_msg_t* msg) {
    _transmitter.send(reinterpret_cast<const std::byte*>(msg), sizeof(ipc_msg_t));
}

template <ITransmiterConcept Transmitter>
void IpcClient<Transmitter>::_init() {
    QWISTYS_TODO_MSG("Handle security stuff")
    _transmitter.connect(IPC_ENDPOINT, 0);
}

#endif  // IPC_CLIENT_H