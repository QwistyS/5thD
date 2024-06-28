#ifndef IZMQ_H
#define IZMQ_H

#include "5thderror_handler.h"
#include "keys.h"
#define ZMQWRAPPER_ID "ZMQWRAPPER_ID"

std::string get_id();

/**
 * @brief ZMQ CURVE API for generating the keys
 * @return 0 on success
 */
int generate_keys(keys_t* keys);

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
    ZMQWContext(IError* error) : _context(nullptr), _error(error) { 
        _init(); };
    void* get_context();

private:
    void _close();
    void _set_context();
    void _init();
    void* _context;
    IError* _error;
    std::string _self_id;
};

#endif  // IZMQ_H