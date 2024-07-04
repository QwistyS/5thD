#include <zmq.h>
#include "qwistys_macro.h"
#include "transmitter.h"

ZMQTransmitter::~ZMQTransmitter() {
    close();
}

void ZMQTransmitter::_init() {
    zmq_setsockopt(_socket->get_socket(), ZMQ_IDENTITY, _identity.c_str(), _identity.size());

    _drp.register_recovery_action(ErrorCode::SOCKET_CONNECT_FAIL, [this]() { return _handle_connect(); });
}

VoidResult ZMQTransmitter::_connect(const std::string& ip, int port) {
    std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);
    if (zmq_connect(_socket->get_socket(), endpoint.c_str()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::SOCKET_CONNECT_FAIL, "Failed connect to " + endpoint);
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

void ZMQTransmitter::worker(int* until, void (*cb)(void* socket)) {
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
            cb(_socket->get_socket());
        }
    }
}

void ZMQTransmitter::connect(const std::string& ip, int port) {
    auto ret = _connect(ip, port);
    if (ret.is_err()) {
        WARN("Trying to resolve connection");
    }
}

void ZMQTransmitter::close() {
}

void ZMQTransmitter::send(void* data, size_t data_length) const {
    zmq_msg_t message;
    zmq_msg_init_size(&message, data_length);
    memcpy(zmq_msg_data(&message), data, data_length);
    if(zmq_send(_socket->get_socket(), "", 0, ZMQ_SNDMORE) == -1) {
        WARN("Fail to send identity {}", zmq_strerror(zmq_errno()));
    }

    auto ret = zmq_msg_send(&message, _socket->get_socket(), 0);
    if (ret == -1) {
        WARN("Fail to send data error {}", zmq_strerror(zmq_errno()));
    }
    zmq_msg_close(&message);
}