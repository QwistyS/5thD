#ifndef SOFTWARE_BUS_H
#define SOFTWARE_BUS_H

#include <functional>
#include <string>

#include "5thdbuffer.h"
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "izmq.h"
#include "receiver.h"

class ZMQBus {
public:
    explicit ZMQBus(IReceiver* receiver) : _error(_drp), _router(receiver) { _init(); }
    ~ZMQBus();
    void set_security(const char* pub_key, const char* prv_key);
    void run();
    static void signal_handler(int sign);

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    IReceiver* _router;
    std::unordered_map<int, std::string> _clients;
    std::mutex _clients_mutex;
    static std::atomic<bool> _poll;
    ManagedBuffer<ZMQAllMsg, 10> _msg_buffer = ManagedBuffer<ZMQAllMsg, 10>(init_allmsg, deinit_allmsg);
    void _init();
    void _handle_msg(void* sock);
    VoidResult _recv_message(void* sock, ipc_msg_t* msg);
    VoidResult _send_message(void* sock, const ipc_msg_t* msg, const std::string& identity);
};

#endif  // SOFTWARE_BUS_H
