#ifndef IZMQ_H
#define IZMQ_H

#include "5thderror_handler.h"

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
    virtual ~ZMQWSocket();
    ZMQWSocket(IContext* ctx, int socket_type)
        : _context(ctx), _socket_type(socket_type), _socket(nullptr), _error(_drp) {
        _init();
    };
    void* get_socket() override;

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    IContext* _context;
    void* _socket;
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
    virtual ~ZMQWContext();
    ZMQWContext() : _context(nullptr), _error(_drp) { _init(); };
    void* get_context() override;

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    void* _context;
    void _close();
    Result<void*> _create_context();
    void _init();
    bool _handle_context_create();
};

#endif  // IZMQ_H