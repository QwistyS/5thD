#include <zmq.h>
#include "receiver.h"

ZMQWReceiver::~ZMQWReceiver() {
    close();
    DEBUG("Closed receiver");
}

void ZMQWReceiver::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::FAIL_BIND_SOCKET, [this]() {
        WARN("Trying to recover error Receiver bind");
        return _handle_bind();
    });
}

void ZMQWReceiver::_init() {
    _setup_drp();
}

void ZMQWReceiver::worker(std::atomic<bool>* until, std::function<void(void*)> callback) {
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

VoidResult ZMQWReceiver::_listen() {
    std::string endpoint;
    if (!_endpoint.empty()) {
        endpoint = _endpoint;
    } else {
        endpoint = "tcp://" + _addr + ":" + std::to_string(_port);
    }

    if (zmq_bind(_socket->get_socket(), endpoint.c_str()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::FAIL_BIND_SOCKET, "Failed bind socket", Severity::HIGH);
    }

    DEBUG("Listener opened on {}", endpoint.c_str());

    return Ok();
}

VoidResult ZMQWReceiver::_close() {
    return Ok();
}

bool ZMQWReceiver::_handle_bind() {
    WARN("Binding issue");
    WARN("Endpoint = {}", _endpoint.c_str());
    WARN("Address = {}", _addr.c_str());
    WARN("Port = {}", _port);

    return false;
}

bool ZMQWReceiver::listen() {
    auto ret = _listen();
    if (ret.is_err()) {
        DEBUG("Error {}", ret.error().message());
        return _error.handle_error(ret.error());
    }
    return true;
}

bool ZMQWReceiver::set_curve_server_options(const char* self_pub_key, const char* self_prv_key,
                                           size_t key_length_bytes) {
    bool ret = false;

    if (!self_pub_key || !self_prv_key) {
        WARN("NO encryption set duo the fact the kye is null");
        return ret;
    }

    int as_server = 1;
    int rc = zmq_setsockopt(_socket->get_socket(), ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
    if (rc != 0) {
        ERROR("Failed to set CURVE_SERVER option: {}", zmq_strerror(errno));
        return ret;
    }

    rc = zmq_setsockopt(_socket->get_socket(), ZMQ_CURVE_PUBLICKEY, self_pub_key, key_length_bytes);
    if (rc != 0) {
        ERROR("Failed to set CURVE_PUBLICKEY: {}", zmq_strerror(errno));
        return ret;
    }

    rc = zmq_setsockopt(_socket->get_socket(), ZMQ_CURVE_SECRETKEY, self_prv_key, key_length_bytes);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SECRETKEY: {}", zmq_strerror(errno));
        return ret;
    }

    int has_curve;
    size_t has_curve_size = sizeof(has_curve);
    zmq_getsockopt(_socket->get_socket(), ZMQ_CURVE_SERVER, &has_curve, &has_curve_size);

    if (has_curve) {
        DEBUG("CURVE security is available");
        ret = true;
    } else {
        WARN("CURVE security is not available");
    }

    return ret;
}

bool ZMQWReceiver::set_endpoint(const char* endpoint) {
    if (!endpoint) {
        return false;
    }
    _endpoint = endpoint;
    return !_endpoint.empty();
}

int ZMQWReceiver::set_sockopt(int option_name, const void* option_value, size_t option_len) {
    return zmq_setsockopt(_socket->get_socket(), option_name, option_value, option_len);
}

int ZMQWReceiver::get_sockopt(int option_name, void* option_value, size_t* option_len) {
    return zmq_getsockopt(_socket->get_socket(), option_name, option_value, option_len);
}

void ZMQWReceiver::close() {
    auto ret = _close();
    if (ret.is_err()) {
        _error.handle_error(ret.error());
    }
}
