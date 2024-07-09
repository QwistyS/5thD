#include "transmitter.h"
#include <zmq.h>
#include "qwistys_macro.h"

ZMQTransmitter::~ZMQTransmitter() {
    close();
}

void ZMQTransmitter::_init() {
    zmq_setsockopt(_socket->get_socket(), ZMQ_IDENTITY, _identity.c_str(), _identity.size());

    _drp.register_recovery_action(ErrorCode::SOCKET_CONNECT_FAIL, [this]() { return _handle_connect(); });
}

VoidResult ZMQTransmitter::_connect(const std::string& ip, int port) {
    std::string endpoint;
    if (port == 0) {
        endpoint = ip;
    } else {
        endpoint = "tcp://" + ip + ":" + std::to_string(port);
    }

    if (zmq_connect(_socket->get_socket(), endpoint.c_str()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::SOCKET_CONNECT_FAIL, "Failed connect to " + endpoint);
    }

    // Poll the socket to check if it is connected
    zmq_pollitem_t poll_items[] = {{_socket->get_socket(), 0, ZMQ_POLLOUT, 0}};
    int timeout_ms = 5000;  // 5 seconds, or set your desired timeout value here
    int poll_rc = zmq_poll(poll_items, 1, timeout_ms);

    if (poll_rc == -1) {
        ERROR("zmq_poll failed: %s", zmq_strerror(errno));
        return Err(ErrorCode::FAIL_POLL_SOCKET, "Init Poll on connetc to " + endpoint);

    } else if (poll_rc == 0) {
        ERROR("Connection timeout after %d milliseconds", timeout_ms);
        return Err(ErrorCode::SOCKET_TIMEOUT, "Timeout waiting for srv " + endpoint);
    } else {
        if (poll_items[0].revents & ZMQ_POLLOUT) {
            DEBUG("Connected to {}", endpoint);
        } else {
            ERROR("Unexpected poll event: %d", poll_items[0].revents);
            return Err(ErrorCode::MONKEY, "When tryed to poll on " + endpoint, Severity::HIGH);
        }
    }

    DEBUG("Connected to {}", endpoint);
    return Ok();
}

bool ZMQTransmitter::_handle_connect() {
    ERROR("Fail to connect");
    return false;
}

void ZMQTransmitter::set_curve_client_options(const char* server_public_key) {
}

void ZMQTransmitter::worker(std::atomic<bool>* until, std::function<void(void*)> callback) {
    // Poll items setup
    zmq_pollitem_t items[] = {{_socket->get_socket(), 0, ZMQ_POLLIN, 0}};

    while (*until) {
        // Poll for incoming messages
        int rc = zmq_poll(items, 1, -1);  // -1 means wait indefinitely

        if (rc == -1) {
            ERROR("zmq_poll");
            break;
        }

        // Check if there's incoming data on the dealer socket
        if (items[0].revents & ZMQ_POLLIN) {
            callback(_socket->get_socket());
        }
    }
}

int ZMQTransmitter::set_sockopt(int option_name, const void* option_value, size_t option_len) {
    return zmq_setsockopt(_socket->get_socket(), option_name, option_value, option_len);
}

void ZMQTransmitter::connect(const std::string& ip, int port) {
    auto ret = _connect(ip, port);
    if (ret.is_err()) {
        WARN("Trying to resolve connection");
        _error.handle_error(ret.error());
    }
}

void ZMQTransmitter::close() {
}

void ZMQTransmitter::send(void* data, size_t data_length) const {
    zmq_msg_t message;
    zmq_msg_init_size(&message, data_length);
    memcpy(zmq_msg_data(&message), data, data_length);
    if (zmq_send(_socket->get_socket(), "", 0, ZMQ_SNDMORE) == -1) {
        WARN("Fail to send identity. Error:{}", zmq_strerror(zmq_errno()));
    }

    auto ret = zmq_msg_send(&message, _socket->get_socket(), 0);
    if (ret == -1) {
        WARN("Fail to send data. Error:{}", zmq_strerror(zmq_errno()));
    }
    zmq_msg_close(&message);
}