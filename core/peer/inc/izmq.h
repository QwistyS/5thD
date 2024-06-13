#ifndef IZMQ_H
#define IZMQ_H

#include "5thderror_handler.h"

/**
 * @brief Interface for network library context.
 * @note Currently we use ZMQ but need to check libp2p, also useful for
 * testing mocks.
 */
class IContext {
public:
    virtual ~IContext() = default;
    virtual void* get_context() = 0;
};

/**
 * @brief Wrapper for zmq context
 */
class ZMQWContext : public IContext {
public:
    virtual ~ZMQWContext();
    ZMQWContext(IError *error) : _context(nullptr),  _error(error) { set_context(); };
    void* get_context();

private:
    void close();
    void set_context();
    void* _context;
    IError* _error;
};

#endif  // IZMQ_H