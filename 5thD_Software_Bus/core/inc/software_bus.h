#ifndef SOFTWARE_BUS_H
#define SOFTWARE_BUS_H

#include <string>

#include "5thderror_handler.h"
#include "5thdipcmsg.h"

#define SOFTWARE_BUS_ID "softbus"

class ZMQBus {
public:
    ZMQBus(const char* addr, int timeout, IError* e)
        : _addr(addr), _timeout(timeout), _errors(e), _context(nullptr), _router(nullptr) {
        _init();
    };
    ~ZMQBus();
    void run();

private:
    void* _context;
    void* _router;
    const char* _addr;
    int _timeout;
    IError* _errors;
    void _init();
};

#endif  // SOFTWARE_BUS_H