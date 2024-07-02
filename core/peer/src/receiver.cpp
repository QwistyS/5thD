#include <zmq.h>

#include "5thdlogger.h"
#include "5thdmsg.h"
#include "qwistys_macro.h"
#include "receiver.h"

static int id_counter = 0;

void Receiver::worker() {
    zmq_pollitem_t items[] = {{_thread_args.socket, 0, ZMQ_POLLIN, 0}};
    DEBUG("Polling thread started");
    while (_thread_args.active) {
        zmq_poll(items, 1, 1000);  // Poll with a timeout of 1000 ms
        if (items[0].revents & ZMQ_POLLIN) {
            netmsg_t _msg;
            int identity_size = zmq_recv(_socket_rout, _msg.conn_info.id, sizeof(_msg.conn_info.id), 0);
            if (identity_size > 0) {
                DEBUG("Socket identity {}", _msg.conn_info.id);
            }

            // Receive the delimiter frame (an empty frame in ROUTER/DEALER pattern)
            zmq_recv(_socket_rout, NULL, 0, 0);

            int message_size = zmq_recv(_socket_rout, _msg.msg, sizeof(_msg.msg), 0);

            DEBUG("Received: {}", _msg.msg);
            QWISTYS_TODO_MSG("Shove the messages into q");
        }
    }
}

Receiver::~Receiver() {
    _thread_args.active = false;
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }
    close();
    _error->unreg(_self_id);
    DEBUG("Receiver {} done", _self_id);
}

void Receiver::listen() {
    _error->handle(_self_id, Errors::OK);
    std::string endpoint = "tcp://" + _addr + ":" + std::to_string(_port);

    int bind_result = zmq_bind(_socket_rout, endpoint.c_str());
    if (bind_result == Errors::OK) {
        DEBUG("Listener opened on {}", endpoint.c_str());
    } else {
        int err = zmq_errno();
        const char* err_str = zmq_strerror(err);
        ERROR("FAIL to open listener on {}: Error {}: {}", endpoint.c_str(), err, err_str);
        _error->handle(_self_id, Errors::FAIL_BIND_SOCKET);
        return;
    }

    _thread_args.active = true;
    _thread_args.socket = _socket_rout;

    _worker_thread = std::thread(&Receiver::worker, this);
}

void Receiver::_init() {
    _self_id = RECEIVER_ID + std::to_string(id_counter++);
    _error->reg(_self_id);
    _error->handle(_self_id, Errors::OK);

    _socket_rout = zmq_socket(_ctx->get_context(), ZMQ_ROUTER);

    if (_socket_rout == nullptr) {
        ERROR("FAIL to init receiver socket");
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
        _error->handle(_self_id, Errors::SOCKET_INIT_FAIL);
    }
}

void Receiver::set_curve_server_options(const char* server_public_key, const char* server_secret_key) {
    _error->handle(_self_id, Errors::OK);
    int enable_curve = 1;
    // zmq_setsockopt(_socket_rout, ZMQ_CURVE_SERVER, &enable_curve, sizeof(enable_curve));
    if (zmq_setsockopt(_socket_rout, ZMQ_CURVE_PUBLICKEY, server_public_key, KEY_LENGTH) != Errors::OK)
        _error->handle(_self_id, Errors::FAIL_SET_SOCKOPT);

    if (zmq_setsockopt(_socket_rout, ZMQ_CURVE_SECRETKEY, server_secret_key, KEY_LENGTH) != Errors::OK)
        _error->handle(_self_id, Errors::FAIL_SET_SOCKOPT);
}

void Receiver::close() {
    if (_socket_rout) {
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
    }
}