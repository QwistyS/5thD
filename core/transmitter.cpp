#include "transmitter.h"
#include <features.h>
#include <zmq.h>
#include <algorithm>
#include <cstddef>
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

#define DEFAULT_DATA_CHUNK sizeof(ipc_msg_t)

ZMQWTransmitter::~ZMQWTransmitter() {
    close();
}

void ZMQWTransmitter::_init() {
    zmq_setsockopt(_socket->get_socket(), ZMQ_IDENTITY, _identity.c_str(), _identity.size());

    _drp.register_recovery_action(ErrorCode::SOCKET_CONNECT_FAIL, [this]() { return _handle_connect(); });
}

VoidResult ZMQWTransmitter::_connect(const std::string& ip, int port) {
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

    if (int poll_rc = zmq_poll(poll_items, 1, timeout_ms); poll_rc == -1) {
        return Err(ErrorCode::FAIL_POLL_SOCKET, "Init Poll on connect to " + endpoint);

    } else if (poll_rc == 0) {
        return Err(ErrorCode::SOCKET_TIMEOUT, "Timeout waiting for srv " + endpoint);
    } else {
        if (poll_items[0].revents & ZMQ_POLLOUT) {
            DEBUG("Connected to {}", endpoint);
        } else {
            return Err(ErrorCode::MONKEY, "Unexpected stuff on poll event from zmq " + endpoint, Severity::HIGH);
        }
    }

    return Ok();
}

void ZMQWTransmitter::set_curve_client_options(const char* server_public_key) {
    QWISTYS_UNIMPLEMENTED();
}

void ZMQWTransmitter::worker(std::atomic<bool>* until, const std::function<void(void*)>& callback) {
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

int ZMQWTransmitter::set_sockopt(int option_name, const void* option_value, size_t option_len) {
    return zmq_setsockopt(_socket->get_socket(), option_name, option_value, option_len);
}

bool ZMQWTransmitter::connect(const std::string& ip, int port) {
    if (auto ret = _connect(ip, port); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

void ZMQWTransmitter::close() {
    WARN("If you need to close the transmitter object, then release/delete it");
}

bool ZMQWTransmitter::send(const std::byte* data, size_t data_length) {
    if (auto ret = _send(data, data_length); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

VoidResult ZMQWTransmitter::_send(const std::byte* data, size_t num_bytes) {
    int rc;
    size_t min = 0;

    if (_msg_buffer.is_full().value()) {
        return Err(ErrorCode::MANAGE_BUFF_FULL, "messages buffer is full", Severity::LOW);
    }
    auto all_msg = _msg_buffer.get_slot();

    // RAII cleanup
    auto buffer_cleanup = [this](decltype(all_msg)* msg_slot) { _msg_buffer.release_slot(msg_slot); };
    std::unique_ptr<decltype(all_msg), decltype(buffer_cleanup)> all_msg_ptr(&all_msg, buffer_cleanup);

    if (!all_msg) {
        WARN("Fail to allocate slot");
        return Err(ErrorCode::MANAGE_BUFF_MONKEY, "IDK what happen :/", Severity::MEDIUM);
    }

    // Ensure identity is valid
    if (_identity.empty()) {
        return Err(ErrorCode::INVALID_IDENTITY, "Identity is empty");
    }
    // Send identity
    memcpy(zmq_msg_data(&all_msg->identity), _identity.c_str(), _identity.size());
    rc = zmq_msg_send(&all_msg->identity, _socket->get_socket(), ZMQ_SNDMORE);

    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Failed to send identity frame");
    }

    // Send empty
    rc = zmq_msg_send(&all_msg->empty, _socket->get_socket(), ZMQ_SNDMORE);
    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Failed to send empty frame");
    }

    while (num_bytes > 0) {
        min = std::min(num_bytes, DEFAULT_DATA_CHUNK);

        zmq_msg_init_size(&all_msg->msg, min);
        memcpy(zmq_msg_data(&all_msg->msg), data, min);
        rc = zmq_msg_send(&all_msg->msg, _socket->get_socket(), (num_bytes > min) ? ZMQ_SNDMORE : 0);
        DEBUG("Sent {} bytes", rc);
        if (rc == -1) {
            zmq_msg_close(&all_msg->msg);
            return Err(ErrorCode::FAIL_SEND_FRAME, "Failed to send chunk of data");
        }
        zmq_msg_close(&all_msg->msg);

        data += min;
        num_bytes -= min;
    }
    return Ok();
}

bool ZMQWTransmitter::_handle_connect() const {
    WARN("Trying to resolve connect");
    return false;
}

bool ZMQWTransmitter::_handle_msg_buff() const {
    if (auto ret = _msg_buffer.check_integrity(); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

void ZMQWTransmitter::_setup_drp() {
    // clang-format off
    _drp.register_recovery_action(ErrorCode::SOCKET_CONNECT_FAIL, 
        [this]() { 
            return _handle_connect(); 
    });
    _drp.register_recovery_action(ErrorCode::MANAGE_BUFF_FULL, 
        [this]() { 
            return _handle_msg_buff();
    });
    // clang-format on
}
