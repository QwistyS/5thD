#ifndef SOFTWARE_BUS_H
#define SOFTWARE_BUS_H

#include <functional>
#include <string>
#include "5htdbuffer.h"
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "receiver.h"

struct ZMQAllMsg {
    zmq_msg_t identity;
    zmq_msg_t empty;
    zmq_msg_t msg;
};

void init_allmsg(ZMQAllMsg& msg);

void deinit_allmsg(ZMQAllMsg& msg);

class ZMQBus {
public:
    ZMQBus(IReceiver* receiver)
        : _router(receiver),
          _error(_drp),
          _msg_buffer(init_allmsg, deinit_allmsg) {
        _init();
    }
    ~ZMQBus();
    void set_security(const char* pub_key, const char* prv_key);
    void run();
    static void signal_handler(int sign);

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    IReceiver* _router;
    std::unordered_map<int, std::string> _clients;
    std::mutex _clients_mutex;
    static std::atomic<bool> _poll;
    ManagedBuffer<ZMQAllMsg, 10> _msg_buffer;
    void _init();
    void _handle_msg(void* sock);
    VoidResult _recv_message(void* sock, ipc_msg_t* msg);
    VoidResult _send_message(void* sock, const ipc_msg_t* msg, const std::string& identity);
};

#endif  // SOFTWARE_BUS_H
