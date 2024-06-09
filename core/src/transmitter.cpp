#include "transmitter.h"
#include "qwistys_macro.h"
#include <spdlog/spdlog.h>

#define DEBUG(x, ...) spdlog::info(x, ##__VA_ARGS__)

Transmiter::~Transmiter() {
    close();
    _context.close();
}

void Transmiter::connect(const std::string &ip, int port) {
    _socket = std::make_unique<zmq::socket_t>(_context, zmq::socket_type::req);
    auto ret = zmq_setsockopt(_socket.get(), ZMQ_LINGER, "", 0);
    std::string addr = "tcp://" + ip + ":" + std::to_string(port);
    DEBUG("Connecting to addr {}", addr);

    try {
        _socket->connect(addr.c_str());
        _connected = is_connected();
    } catch (const zmq::error_t& e) {
        DEBUG("ZMQ Error: {}", e.what());
    } catch (const std::exception& e) {
        DEBUG("Standard Exception: {}", e.what());
    } catch (...) {
        DEBUG("Unknown Error");
    }

    DEBUG("Connection status to addr {} = {}", addr, _connected);
}

void Transmiter::close() {
    if (_connected) _socket->close();
}

void Transmiter::send(void *data) {
}

bool Transmiter::is_connected() {
    zmq::message_t request(9);
    zmq::message_t reply;

    memcpy(request.data(), "Heartbeat", 9);
    _socket->send(request, zmq::send_flags::none);

    auto ret =_socket->recv(reply, zmq::recv_flags::none);
    std::string reply_str(static_cast<char*>(reply.data()), reply.size());
    _connected = (reply_str == "Alive");

    return _connected;
}
