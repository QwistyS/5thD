#include "5thderrors.h"
#include "5thdlogger.h"
#include "keys.h"
#include "qwistys_macro.h"
#include "transmitter.h"

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

void ZMQTransmitter::_init(const std::string& id) {
    _error->reg(id);
    _error->handle(id, Errors::OK);

    memcpy(&_self_info.id, id.c_str(), id.size());

    _socket = zmq_socket(_context->get_context(), _socket_type);

    if (_socket == nullptr) {
        ERROR("Fail to init socket");
        _error->handle(_self_info.id, Errors::SOCKET_INIT_FAIL);
    }

    if (zmq_setsockopt(_socket, ZMQ_IDENTITY, _self_info.id, sizeof(_self_info.id)) != Errors::OK)
        _error->handle(_self_info.id, Errors::FAIL_SETSOCKOPT_ID);
}

void ZMQTransmitter::connect(const std::string& ip, int port) {
    _error->handle(_self_info.id, Errors::OK);

    _self_info.addr = ip;
    _self_info.port = port;

    std::string addr = "tcp://" + ip + ":" + std::to_string(port);
    DEBUG("Connecting to addr {}", addr);

    if (zmq_connect(_socket, addr.c_str()) == Errors::OK) {
        DEBUG("Connected to {}", addr);
    } else {
        ERROR("Fail to connect to {}", addr);
        zmq_close(_socket);
        _socket = nullptr;
        _error->handle(_self_info.id, Errors::SOCKET_CONNECT_FAIL);
    }
}

void ZMQTransmitter::close() {
    _error->handle(_self_info.id, Errors::OK);

    if (zmq_close(_socket) == Errors::OK) {
        DEBUG("ZMQ closed socket");
    } else {
        int errnum = zmq_errno();
        ERROR("ZMQ Fail to close socket with error {}", zmq_strerror(errnum));
        _error->handle(_self_info.id, Errors::SOCKET_CLOSE_FAIL);
    }
    _socket = nullptr;
}

void ZMQTransmitter::send(void* data, size_t data_length) const {
    QWISTYS_UNIMPLEMENTED();
}

void ZMQTransmitter::send_stream(void* data, size_t data_length, int chunk_size) {
    _error->handle(_self_info.id, Errors::OK);
    uint8_t* pdata = static_cast<uint8_t*>(data);
    int flags = 0;

    while (data_length > 0) {
        size_t current_chunk_size = (data_length < chunk_size) ? data_length : chunk_size;
        flags = (data_length > current_chunk_size) ? ZMQ_SNDMORE : 0;

        if (zmq_send(_socket, pdata, current_chunk_size, flags) != Errors::OK) {
            pdata = nullptr;
            ERROR("ZMQ failed to send data chunk");
            _error->handle(_self_info.id, Errors::SOCKET_SEND_FAIL);
            return;
        }

        pdata += current_chunk_size;
        data_length -= current_chunk_size;
    }

    // Send an empty message to signify the end
    if (zmq_send(_socket, "", 0, 0) < 0) {
        ERROR("ZMQ failed to send final empty message");
        _error->handle(_self_info.id, Errors::SOCKET_SEND_FAIL);
    }
}