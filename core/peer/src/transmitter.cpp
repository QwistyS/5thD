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
    QWISTYS_TODO_MSG("Handle the case when zmq cant open socket");
    _socket = zmq_socket(_context->get_context(), _socket_type);
    if (_socket == NULL) {
        ERROR("ZMQ Fail to create socket");
    } else {
        DEBUG("Socket created successfully");
    }

    std::string addr = "tcp://" + ip + ":" + std::to_string(port);
    DEBUG("Connecting to addr {}", addr);

    if (zmq_connect(_socket, addr.c_str()) == Errors::OK) {
        DEBUG("Connected to {}", addr);
    } else {
        ERROR("Fail to connect to {}", addr);
        zmq_close(_socket);
        _socket = nullptr;
        _error_handler->handle(Errors::SOCKET_INIT_FAIL);
    }
}

void Transmitter::close() {
    if (zmq_close(_socket) == Errors::OK) {
        DEBUG("ZMQ closed socket");
    } else {
        int errnum = zmq_errno();
        ERROR("ZMQ Fail to close socket with error {}", zmq_strerror(errnum));
        _error_handler->handle(Errors::SOCKET_CLOSE_FAIL);
    }
    _socket = nullptr;
}

void Transmitter::send(void* data, size_t data_length) const {
    QWISTYS_UNIMPLEMENTED();
}

void Transmitter::send_stream(void* data, size_t data_length, int chunk_size) {
    uint8_t* pdata = static_cast<uint8_t*>(data);
    int flags = 0;

    while (data_length > 0) {
        size_t current_chunk_size = (data_length < chunk_size) ? data_length : chunk_size;
        flags = (data_length > current_chunk_size) ? ZMQ_SNDMORE : 0;

        if (zmq_send(_socket, pdata, current_chunk_size, flags) != Errors::OK) {
            pdata = nullptr;
            ERROR("ZMQ failed to send data chunk");
            _error_handler->handle(Errors::SOCKET_SEND_FAIL);
            return;
        }

        pdata += current_chunk_size;
        data_length -= current_chunk_size;
    }

    // Send an empty message to signify the end
    if (zmq_send(_socket, "", 0, 0) < 0) {
        ERROR("ZMQ failed to send final empty message");
        _error_handler->handle(Errors::SOCKET_SEND_FAIL);
    }
}

bool Transmitter::req_data(const char* OP) const {
    zmq_msg_t request;
    zmq_msg_t reply;
    bool ret = false;

    if (_socket == nullptr) {
        DEBUG("NO VALID SOCKET");
        return ret;
    }

    if (zmq_msg_init_size(&request, strlen(OP)) == 0) {
        memcpy(zmq_msg_data(&request), OP, strlen(OP));
        DEBUG("Msg init success");

        // Send the heartbeat message
        if (zmq_msg_send(&request, _socket, 0) == strlen(OP)) {
            DEBUG("Message {} {} sent", OP, strlen(OP));

            // Initialize the reply message
            if (zmq_msg_init(&reply) == Errors::OK) {
                // Receive the reply

                auto zmq_e = zmq_msg_recv(&reply, _socket, 0);

                if (zmq_e != -1) {
                    std::string reply_str(static_cast<char*>(zmq_msg_data(&reply)), zmq_msg_size(&reply));
                    ret = (reply_str == "Alive");
                    DEBUG("Received reply: {}", reply_str);
                } else {
                    ERROR("FAIL receive {}", zmq_strerror(zmq_e));
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
