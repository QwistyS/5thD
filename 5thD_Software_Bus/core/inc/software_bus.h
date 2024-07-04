#ifndef SOFTWARE_BUS_H
#define SOFTWARE_BUS_H

#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "receiver.h"

class ZMQBus {
public:
    ZMQBus(IReceiver* receiver) : _router(receiver), _error(_drp), _poll(true) { _init(); };
    ~ZMQBus();
    void run();

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    IReceiver* _router;
    std::unordered_map<int, std::string> _clients;
    std::mutex _clients_mutex;
    std::atomic<bool> _poll;
    void _init();
    void _handle_msg(void* sock);
};

#endif  // SOFTWARE_BUS_H