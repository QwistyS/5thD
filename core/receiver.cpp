#include <zmq.h>

#include "qwistys_macro.h"
#include "receiver.h"

void erro_cb(const Error& e) {
    WARN("Trying to resolve {}", e.message());
}

ZMQReceiver::~ZMQReceiver() {
    close();
}

void ZMQReceiver::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::FAIL_BIND_SOCKET, [this]() {
        WARN("Trying to recover error Receiver bind");
        return _handle_bind_issue();
    });
}

void ZMQReceiver::_init() {
    _setup_drp();
    _error.register_callback(ErrorCode::FAIL_BIND_SOCKET, erro_cb);
}

void ZMQReceiver::worker(std::atomic<bool>* until, std::function<void(void*)> callback) {
    zmq_pollitem_t items[] = {{_socket->get_socket(), 0, ZMQ_POLLIN, 0}};

    DEBUG("Polling thread started");

    while (*until) {
        auto ret = zmq_poll(items, 1, 500);  // Poll with 500 ms timeout

        if (ret == (int) ErrorCode::OK) {
            DEBUG("Timeout reached");
            continue;
        } else if (ret == -1) {
            ERROR("Error in zmq_poll {}", zmq_strerror(zmq_errno()));
            break;
        } else {
            if (items[0].revents & ZMQ_POLLIN) {
                DEBUG("Message received");
                callback(_socket->get_socket());
            }
        }
    }
    DEBUG("Polling ended");
}

VoidResult ZMQReceiver::_listen() {
    std::string endpoint;
    if (!_endpoint.empty()) {
        endpoint = _endpoint;
    } else {
        endpoint = "tcp://" + _addr + ":" + std::to_string(_port);
    }

    if (zmq_bind(_socket->get_socket(), endpoint.c_str()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::FAIL_BIND_SOCKET, "Failed bind socket");
    }

    DEBUG("Listener opened on {}", endpoint.c_str());

    return Ok();
}

VoidResult ZMQReceiver::_close() {
    if (zmq_close(_socket->get_socket()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::FAIL_CLOSE_ZQM_SOCKET, "Fail to close transmitter socket");
    }
    return Ok();
}

bool ZMQReceiver::_handle_bind_issue() {
    auto ret = _listen();
    if (ret.is_err()) {
        return false;
    }
    return true;
}

void ZMQReceiver::listen() {
    auto ret = _listen();
    if (ret.is_err()) {
        _error.handle_error(ret.error());
    }
}

void ZMQReceiver::set_curve_server_options(const char* server_public_key, const char* server_secret_key,
                                           size_t key_length_bytes) {
    int enable_curve = 1;
    zmq_setsockopt(_socket->get_socket(), ZMQ_CURVE_SERVER, &enable_curve, sizeof(enable_curve));
    if (zmq_setsockopt(_socket->get_socket(), ZMQ_CURVE_PUBLICKEY, server_public_key, key_length_bytes)
        != (int) ErrorCode::OK) {
    }

    if (zmq_setsockopt(_socket->get_socket(), ZMQ_CURVE_SECRETKEY, server_secret_key, key_length_bytes)
        != (int) ErrorCode::OK) {
    }
}

void ZMQReceiver::set_endpoint(const char* endpoint) {
    if (!endpoint) {
        PANIC("No end point provided for receiver");
    }
    _endpoint = endpoint;
}

void ZMQReceiver::close() {
    auto ret = _close();
    if (ret.is_err()) {
        _error.handle_error(ret.error());
    }
}