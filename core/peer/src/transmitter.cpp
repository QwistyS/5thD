#include "keys.h"
#include "transmitter.h"
#include "5thderrors.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

#include <zmq.h>

#define MESSAGES_MAX_AMOUNT 2

static zmq_msg_t request[MESSAGES_MAX_AMOUNT];
static zmq_msg_t replay[MESSAGES_MAX_AMOUNT];

ZMQTransmitter::~ZMQTransmitter() {
    _clear_buffers();
    if (_socket)
        close();
}

void ZMQTransmitter::_clear_buffers() {
    for (int i = 0; i < MESSAGES_MAX_AMOUNT; i++) {
        zmq_msg_close(&replay[i]);
        zmq_msg_close(&request[i]);
    }
}

void ZMQTransmitter::set_curve_client_options(const char* server_public_key) {
    auto keys = Keys::get_instance();

    zmq_setsockopt(_socket, ZMQ_CURVE_PUBLICKEY, keys->get_key(PUBLIC_KEY_FLAG), KEY_LENGTH);
    zmq_setsockopt(_socket, ZMQ_CURVE_SECRETKEY, keys->get_key(PRIVATE_KEY_FLAG), KEY_LENGTH);
    zmq_setsockopt(_socket, ZMQ_CURVE_SERVERKEY, server_public_key, KEY_LENGTH);
}

void ZMQTransmitter::_init() {
    QWISTYS_TODO_MSG("Create a normal way to handle buffers");
    for (int i = 0; i < MESSAGES_MAX_AMOUNT; i++) {
        zmq_msg_init(&replay[i]);
        if (zmq_msg_init_size(&replay[i], strlen(GENERIC_DATA)) != Errors::OK) {
            ERROR("FAIL to init message size");
        }
        zmq_msg_init(&request[i]);
        if (zmq_msg_init_size(&request[i], strlen(GENERIC_DATA)) != Errors::OK) {
            ERROR("FAIL to init message size");
        }
    }
    QWISTYS_TODO_MSG("Handle the case when zmq cant open socket");
    _socket = zmq_socket(_context->get_context(), _socket_type);
    if (_socket == NULL) {
        ERROR("ZMQ Fail to create socket");
    } else {
        DEBUG("Socket created successfully");
    }
}

void ZMQTransmitter::connect(const std::string& ip, int port) {
    std::string addr = "tcp://" + ip + ":" + std::to_string(port);
    DEBUG("Connecting to addr {}", addr);

    QWISTYS_TODO_MSG("Handle the case when zmq cant open socket");
    if (zmq_connect(_socket, addr.c_str()) == Errors::OK) {
        DEBUG("Connected to {}", addr);
    } else {
        ERROR("Fail to connect to {}", addr);
        zmq_close(_socket);
        _socket = nullptr;
        _error_handler->handle(Errors::SOCKET_CONNECT_FAIL);
    }
}

void ZMQTransmitter::close() {
    if (zmq_close(_socket) == Errors::OK) {
        DEBUG("ZMQ closed socket");
    } else {
        int errnum = zmq_errno();
        ERROR("ZMQ Fail to close socket with error {}", zmq_strerror(errnum));
        _error_handler->handle(Errors::SOCKET_CLOSE_FAIL);
    }
    _socket = nullptr;
}

void ZMQTransmitter::send(void* data, size_t data_length) const {
    QWISTYS_UNIMPLEMENTED();
}

void ZMQTransmitter::send_stream(void* data, size_t data_length, int chunk_size) {
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

bool ZMQTransmitter::req_data(const char* OP) const {
    bool ret = false;

    if (_socket == nullptr) {
        DEBUG("NO VALID SOCKET");
        return ret;
    }

    memcpy(zmq_msg_data(&request[0]), OP, strlen(OP));

    if (zmq_msg_send(&request[0], _socket, 0) == strlen(OP)) {
        DEBUG("Message {} {} sent", OP, strlen(OP));

        auto zmq_e = zmq_msg_recv(&replay[0], _socket, 0);

        if (zmq_e != -1) {
            std::string reply_str(static_cast<char*>(zmq_msg_data(&replay[0])), zmq_msg_size(&replay[0]));
            ret = (reply_str == "Alive");
            DEBUG("Received reply: {}", reply_str);
        } else {
            ERROR("FAIL receive {}", zmq_strerror(zmq_e));
        }
    } else {
        ERROR("FAIL to send request");
        _error_handler->handle(Errors::SEND_HEARTBEAT_FAIL);
    }
    return ret;
}
