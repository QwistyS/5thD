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

int generate_keys(char* public_key, char* private_key);

template <typename T>
concept ContextConcept = requires(T t) {
    { t.get_context() } -> std::same_as<void*>;
    { t.close() } -> std::same_as<bool>;
};

class ZMQWContext {
public:
    ~ZMQWContext();
    ZMQWContext() : _error(_drp) { _init(); }
    void* get_context();
    bool close();

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    void* _context = nullptr;
    VoidResult _close();
    Result<void*> _create_context();
    void _init();
    bool _handle_context_create();
};

template <typename T>
concept SocketConcept = requires(T t) {
    { t.get_socket() } -> std::same_as<void*>;
};

template <ContextConcept Context>
class ZMQWSocket {
public:
    ~ZMQWSocket();
    ZMQWSocket(Context& ctx, int socket_type) : _error(_drp), _context(ctx), _socket_type(socket_type) { _init(); }
    void* get_socket();

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    Context& _context;
    void* _socket = nullptr;
    int _socket_type;

    void _init();
    Result<void*> _create_socket(int type);
    void _close();
    bool _handle_socket_create();
};

inline ZMQWContext::~ZMQWContext() {
    DEBUG("Close context");
}

inline bool ZMQWContext::close() {
    if (auto ret = _close(); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

inline void* ZMQWContext::get_context() {
    return _context;
}

inline void ZMQWContext::_init() {
    _drp.register_recovery_action(ErrorCode::FAIL_OPEN_ZMQ_CTX, [this]() {
        WARN("Trying to resolve zmq context create error");
        return _handle_context_create();
    });

    auto ret = _create_context();
    if (ret.is_err()) {
        WARN("Error creating zmq context");
    }
}

inline bool ZMQWContext::_handle_context_create() {
    _context = nullptr;

    _context = zmq_ctx_new();
    if (!_context) {
        ERROR("Fail to create zmq context");
        return false;
    }
    return false;
}

inline VoidResult ZMQWContext::_close() {
    if (zmq_ctx_destroy(_context) != (int) ErrorCode::OK) {
        return Err(ErrorCode::FAIL_CLOSE_ZQM_CTX, "Fail to close zmq context");
    }
    _context = nullptr;
    return Ok();
}

inline Result<void*> ZMQWContext::_create_context() {
    _context = zmq_ctx_new();
    if (!_context) {
        return Err<void*>(ErrorCode::FAIL_OPEN_ZMQ_CTX, "Failed to create ZMQ context");
    }
    return Ok<void*>(_context);
}

template <ContextConcept Context>
inline ZMQWSocket<Context>::~ZMQWSocket() {
    _close();
    DEBUG("Closed zmq socket");
}

template <ContextConcept Context>
inline void* ZMQWSocket<Context>::get_socket() {
    return _socket;
}

template <ContextConcept Context>
inline void ZMQWSocket<Context>::_close() {
    if (zmq_close(_socket) != (int) ErrorCode::OK) {
        WARN("Fail to close socket !!!");
    }
    _socket = nullptr;
}

template <ContextConcept Context>
inline bool ZMQWSocket<Context>::_handle_socket_create() {
    _socket = nullptr;

    _socket = zmq_socket(_context.get_context(), _socket_type);
    if (!_socket) {
        switch (errno) {
            case EINVAL:
                ERROR("The requested socket type is invalid.");
                break;
            case EFAULT:
                ERROR("The provided context is invalid.");
                break;
            case EMFILE:
                ERROR("The limit on the total number of open ØMQ sockets has been reached.");
                break;
            case ETERM:
                ERROR("The requested socket type is invalid.");
                break;
            default:
                ERROR("ØMQ monkey error");
                break;
        }
        return false;
    }
    return true;
}

template <ContextConcept Context>
inline void ZMQWSocket<Context>::_init() {
    auto ret = _create_socket(_socket_type);
    if (ret.is_err()) {
        ERROR("Fail to create socket");
    }
}

template <ContextConcept Context>
inline Result<void*> ZMQWSocket<Context>::_create_socket(int type) {
    _socket = zmq_socket(_context.get_context(), _socket_type);
    if (!_socket) {
        return Err<void*>(ErrorCode::FAIL_OPEN_SOCKET, "Failed to create ZMQ socket");
    }
    return Ok<void*>(_socket);
}

#endif  // IZMQ_H
