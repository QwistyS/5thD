#include "5thdipc_client.h"

IpcClient::~IpcClient() {
    _poll = 0;
    if(_worker_thread.joinable()) {
        _worker_thread.join();
    }
}

void IpcClient::send(const ipc_msg_t* msg) {
    _transmitter->send((void*)msg, sizeof(ipc_msg_t));
}

void IpcClient::_init() {
    _transmitter->connect(IPC_ROUTER_ADDR, IPC_ROUTER_PORT);
}
