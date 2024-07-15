#ifndef IZMQ_H
#define IZMQ_H

#include <zmq.h>
#include "5thderror_handler.h"

struct ZMQAllMsg {
    zmq_msg_t identity;
    zmq_msg_t empty;
    zmq_msg_t msg;
};

void init_allmsg(ZMQAllMsg& msg);

void deinit_allmsg(ZMQAllMsg& msg);

/**
 * @brief ZMQ CURVE API for generating the keys
 * @return 0 on success
 */
int generate_keys(char* public_key, char* private_key);

/**
 * @brief Interface for network library context.
 * @note Currently we use ZMQ but need to check libp2p, also useful for
 * testing mocks.
 */
class IContext {
public:
    virtual ~IContext() = default;
    virtual void* get_context() = 0;
    virtual bool close() = 0;
};

/**
 * @brief Interface for network library socket.
 * @note Currently we use ZMQ but need to check libp2p, also useful for
 * testing mocks.
 */
class ISocket {
public:
    virtual ~ISocket() = default;
    virtual void* get_socket() = 0;
};

/**
 * @brief ZMQ Wrapper for socket
 */
class ZMQWSocket : public ISocket {
public:
    ~ZMQWSocket() override;
    ZMQWSocket(IContext* ctx, int socket_type)
        : _error(_drp), _context(ctx), _socket_type(socket_type) {
        _init();
    };
    void* get_socket() override;

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    IContext* _context;
    void* _socket = nullptr;
    int _socket_type;

    void _init();
    Result<void*> _create_socket(int type);
    void _close();
    bool _handle_socket_create();
};

/**
 * @brief Wrapper for zmq context
 */
class ZMQWContext : public IContext {
public:
    ~ZMQWContext() override;
    ZMQWContext() : _error(_drp) { _init(); };
    void* get_context() override;
    bool close() override;


private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    void* _context = nullptr;
    VoidResult _close();
    Result<void*> _create_context();
    void _init();
    bool _handle_context_create();
};

#endif  // IZMQ_H
