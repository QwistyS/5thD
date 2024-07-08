#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include <thread>
#include <string>
#include "transmitter.h"
#include "5thdipcmsg.h"

class IpcClient {
public:
    IpcClient(ITransmitter *transmitter)
        : _transmitter(transmitter), _error(_drp), _poll(1) {
        _init();
    };
    ~IpcClient();
    void send(const ipc_msg_t* msg);
protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
private:
    ITransmitter* _transmitter;
    std::thread _worker_thread;
    int _poll;
    void _init();
};

#endif  // IPC_CLIENT_H