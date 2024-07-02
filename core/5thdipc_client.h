#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include <thread>
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "izmq.h"

class IpcClient {
public:
    IpcClient(void* ctx, std::string id, IError* e)
        : _context(ctx), _self_id(id), _error(e), _socket(nullptr), _poll(true) {
        _init();
    };
    ~IpcClient();
    void send(const ipc_msg_t* msg);

private:
    void* _socket;
    void* _context;
    IError* _error;
    std::string _self_id;
    std::thread _worker_thread;
    bool _poll;
    void _worker();
    void _init();
};

#endif  // IPC_CLIENT_H