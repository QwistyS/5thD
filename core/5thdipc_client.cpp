#include "5thdipc_client.h"
#include <cstddef>

IpcClient::~IpcClient() {
    _poll = 0;
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }
}

void IpcClient::send(const ipc_msg_t* msg) {
    _transmitter->send(reinterpret_cast<const std::byte*>(msg), sizeof(ipc_msg_t));
}

void IpcClient::_init() {
    QWISTYS_TODO_MSG("Handle security stuff")
    _transmitter->connect(IPC_ENDPOINT, 0);
}
