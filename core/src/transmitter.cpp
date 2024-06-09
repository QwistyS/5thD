#include "transmitter.h"
#include "lggr.h"
#include "qwistys_macro.h"

#include <zmq.h>

Transmiter::~Transmiter() {
    close();
}

void Transmiter::connect(const std::string& ip, int port) {
    _socket = zmq_socket(_ctx->get_context(), ZMQ_REQ);
    std::string addr = "tcp://" + ip + ":" + std::to_string(port);
    DEBUG("Connecting to addr {}", addr);

    if (zmq_connect(_socket, addr.c_str()) == 0) {
        DEBUG("Connected to {}", addr);
    } else {
        ERROR("Fail to connect to {}", addr);
    }
}

void Transmiter::close() {
    zmq_close(_socket);
}

void Transmiter::send(void* data) {
}

bool Transmiter::is_connected() {
    zmq_msg_t request;
    zmq_msg_t reply;
    auto ret = false;

    if (zmq_msg_init_size(&request, 9) == 0) {
        memcpy(&request, "Heartbeat", 9);
        DEBUG("Msg init success");
        ret = true;
    } else {
        ERROR("Msg init fail");
    }

    // _socket->send(request, zmq::send_flags::none);

    // auto ret =_socket->recv(reply, zmq::recv_flags::none);
    // std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    // _connected = (reply_str == "Alive");

    return ret;
}
