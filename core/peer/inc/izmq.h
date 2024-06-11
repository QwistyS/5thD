#ifndef IZMQ_H
#define IZMQ_H

#include "transmitter.h"

/**
 * @brief Wrapper for zmq context
 */
class ZMQWContext : public ITransmitterContext {
public:
    virtual ~ZMQWContext();
    ZMQWContext() : _context(nullptr) { set_context(); };
    void* get_context();

private:
    void close();
    void set_context();
    void* _context;
};

#endif  // IZMQ_H