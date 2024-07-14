#include <errno.h>
#include <zmq.h>

#include "5thderror_handler.h"
#include "izmq.h"
#include "5thdipcmsg.h"

void init_allmsg(ZMQAllMsg& msg) {
    zmq_msg_init_size(&msg.identity, strlen(CLIENTS_IDS[0]));
    zmq_msg_init_size(&msg.empty, 0);
    zmq_msg_init_size(&msg.msg, sizeof(ipc_msg_t));
}

void deinit_allmsg(ZMQAllMsg& msg) {
    zmq_msg_close(&msg.empty);
    zmq_msg_close(&msg.identity);
    zmq_msg_close(&msg.msg);
}


int generate_keys(char* public_key, char* private_key) {
    return zmq_curve_keypair(public_key, private_key);
}

// ================ Context wrapper implementation ================

ZMQWContext::~ZMQWContext() {
    _close();
}

void* ZMQWContext::get_context() {
    return _context;
}

void ZMQWContext::_init() {
    _drp.register_recovery_action(ErrorCode::FAIL_OPEN_ZMQ_CTX, [this]() {
        WARN("Trying to resolve zmq context create error");
        return _handle_context_create();
    });

    auto ret = _create_context();
    if (ret.is_err()) {
        WARN("Error creating zmq context");
    }
}

bool ZMQWContext::_handle_context_create() {
    _context = nullptr;

    _context = zmq_ctx_new();
    if (!_context) {
        ERROR("Fail to create zmq context");
        return false;
    }
    return false;
}

void ZMQWContext::_close() {
    if (zmq_ctx_destroy(_context) != (int) ErrorCode::OK) {
        ERROR("Fail to close zmq context");
    }
    _context = nullptr;
    DEBUG("Closed zmq ctx");
}

Result<void*> ZMQWContext::_create_context() {
    _context = zmq_ctx_new();
    if (!_context) {
        return Err<void*>(ErrorCode::FAIL_OPEN_ZMQ_CTX, "Failed to create ZMQ context");
    }
    return Ok<void*>(_context);
}

// ================ Socket wrapper implementation ================

ZMQWSocket::~ZMQWSocket() {
    _close();
    DEBUG("Closed zmq socket");
}

void* ZMQWSocket::get_socket() {
    return _socket;
}

void ZMQWSocket::_close() {
    if (zmq_close(_socket) != (int) ErrorCode::OK) {
        WARN("Fail to close socket !!!");
    }
    _socket = nullptr;
}

bool ZMQWSocket::_handle_socket_create() {
    _socket = nullptr;

    _socket = zmq_socket(_context->get_context(), _socket_type);
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

void ZMQWSocket::_init() {
    auto ret = _create_socket(_socket_type);
    if (ret.is_err()) {
        ERROR("Fail to create socket");
    }
}

Result<void*> ZMQWSocket::_create_socket(int type) {
    _socket = zmq_socket(_context->get_context(), _socket_type);
    if (!_socket) {
        return Err<void*>(ErrorCode::FAIL_OPEN_SOCKET, "Failed to create ZMQ socket");
    }
    return Ok<void*>(_socket);
}
