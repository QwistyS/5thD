#include <zmq.h>

#include "5thdlogger.h"
#include "qwistys_macro.h"
#include "receiver.h"

void* worker(void* args) {
    thread_args_t* _args = (thread_args_t*) args;

    zmq_pollitem_t items[] = {{_args->socket, 0, ZMQ_POLLIN, 0}};
    DEBUG("Polling thread started");
    while (_args->active) {
        zmq_poll(items, 1, 1000);  // Poll with a timeout of 1000 ms

        if (items[0].revents & ZMQ_POLLIN) {
            DEBUG("message received");
            zmq_msg_t identity;
            zmq_msg_init(&identity);
            zmq_msg_recv(&identity, _args->socket, 0);
            DEBUG("Socket identity {}", zmq_msg_data(&identity));

            zmq_msg_t message;
            zmq_msg_init(&message);
            zmq_msg_recv(&message, _args->socket, 0);

            DEBUG("Received: {}", zmq_msg_data(&message));

            // Echo the message back to the client
            zmq_msg_send(&identity, _args->socket, ZMQ_SNDMORE);
            zmq_msg_send(&message, _args->socket, 0);

            zmq_msg_close(&identity);
            zmq_msg_close(&message);
        }
    }
    return NULL;
}

Receiver::~Receiver() {
    _thread_args.active = false;
    close();
}

void Receiver::listen() {
    std::string addr = "tcp://*:" + std::to_string(_port);
    if (zmq_bind(_socket_rout, addr.c_str()) == Errors::OK) {
        DEBUG("Listener opened on {}", addr.c_str());
    } else {
        ERROR("FAIL to open listener om {}", addr.c_str());
        _error->handle(Errors::FAIL_BIND_SOCKET);
    }
    _thread_args.active = true;
    _thread_args.socket = _socket_rout;
    if (pthread_create(&_worker_thread, NULL, worker, &_thread_args) != Errors::OK) {
        ERROR("Fail to init thread");
    }
}

void Receiver::_init() {
    std::string addr = "tcp://*:" + std::to_string(_port);
    _socket_rout = zmq_socket(_ctx->get_context(), ZMQ_REP);

    if (_socket_rout == nullptr) {
        ERROR("FAIL to init listener on address {}", addr);
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
        _error->handle(Errors::FAIL_INIT_LISTENER);
    }
}

void Receiver::set_curve_server_options(const char* server_public_key, const char* server_secret_key) {
    int enable_curve = 1;
    // zmq_setsockopt(_socket_rout, ZMQ_CURVE_SERVER, &enable_curve, sizeof(enable_curve));
    // zmq_setsockopt(_socket_rout, ZMQ_CURVE_PUBLICKEY, server_public_key, KEY_LENGTH - 1);
    // zmq_setsockopt(_socket_rout, ZMQ_CURVE_SECRETKEY, server_secret_key, KEY_LENGTH - 1);
}

void Receiver::close() {
    if (_socket_rout) {
        zmq_close(_socket_rout);
        _socket_rout = nullptr;
    }
}