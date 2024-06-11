#include "transmitter.h"
#include "5thderrors.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

#include <zmq.h>

Transmitter::~Transmitter() {
    if (_socket)
        close();
}

void Transmitter::connect(const std::string& ip, int port) {
    _socket = zmq_socket(_ctx->get_context(), ZMQ_REQ);
    if (_socket == NULL) {
        ERROR("ZMQ Fail to create socket");
        QWISTYS_TODO_MSG("Handle the case when zmq cant open socket");
    } else {
        DEBUG("Socket created successfully");
    }

    std::string addr = "tcp://" + ip + ":" + std::to_string(port);
    DEBUG("Connecting to addr {}", addr);

    if (zmq_connect(_socket, addr.c_str()) == Errors::OK) {
        DEBUG("Connected to {}", addr);
    } else {
        ERROR("Fail to connect to {}", addr);
        _socket = nullptr;
        _error_handler->handle(Errors::SOCKET_INIT_FAIL);
    }
}

void Transmitter::close() {
    if (zmq_close(_socket) == Errors::OK) {
        DEBUG("ZMQ closed socket");
    } else {
        ERROR("ZMQ Fail to close socket");
        _error_handler->handle(Errors::SOCKET_CLOSE_FAIL);
    }
    _socket = nullptr;
}

void Transmitter::send(void* data) {
    QWISTYS_UNIMPLEMENTED();
}

bool Transmitter::is_connected() {
    zmq_msg_t request;
    zmq_msg_t reply;
    bool ret = false;

    if (_socket == nullptr) {
        DEBUG("NO VALID SOCKET");
        return ret;
    }
    // Initialize the request message with size 9 (for "Heartbeat")
    if (zmq_msg_init_size(&request, 9) == 0) {
        // Copy "Heartbeat" string to the message data
        memcpy(zmq_msg_data(&request), "Heartbeat", 9);
        DEBUG("Msg init success");

        // Send the heartbeat message
        if (zmq_msg_send(&request, _socket, 0) == 9) {
            DEBUG("Heartbeat sent");

            // Initialize the reply message
            if (zmq_msg_init(&reply) == Errors::OK) {
                // Receive the reply
                if (zmq_msg_recv(&reply, _socket, 0) != -1) {
                    std::string reply_str(static_cast<char*>(zmq_msg_data(&reply)), zmq_msg_size(&reply));
                    ret = (reply_str == "Alive");
                    DEBUG("Received reply: {}", reply_str);
                }
                // Close the reply message to free allocated resources
                zmq_msg_close(&reply);
            }
        }
        // Close the request message to free allocated resources
        zmq_msg_close(&request);
    } else {
        ERROR("Msg init fail");
        _error_handler->handle(Errors::SEND_HEARTBEAT_FAIL);
    }

    return ret;
}
