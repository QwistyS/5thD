#include "5thderrors.h"
#include "5thdipc_client.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"
#include "zmq.h"

IpcClient::~IpcClient() {
    zmq_close(_socket);
    _socket = nullptr;

    _poll = false;
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }

    _error->unreg(_self_id);
}

void IpcClient::_init() {
    if (_error == nullptr) {
        ERROR("IPC client init without error");
        abort();
    }

    _error->reg(_self_id);

    if (_context == nullptr) {
        ERROR("No zmq ctx provided");
        _error->handle(_self_id, Errors::FAIL_OPEN_ZMQ_CTX);
        abort();
    }

    _socket = zmq_socket(_context, ZMQ_DEALER);
    if (_socket == nullptr) {
        ERROR("{} Fail to init socket", _self_id);
        _error->handle(_self_id, Errors::FAIL_OPEN_SOCKET);
        abort();
    }

    if (zmq_setsockopt(_socket, ZMQ_IDENTITY, _self_id.c_str(), _self_id.size()) != Errors::OK) {
        ERROR("{} Fail to setsockopt", _self_id);
        _error->handle(_self_id, FAIL_SET_SOCKOPT);
    }

    if (zmq_connect(_socket, IPC_ROUTER_ADDR) != Errors::OK) {
        ERROR("{} Fail connect to ipc router", _self_id);
        _error->handle(_self_id, Errors::SOCKET_CONNECT_FAIL);
    }

    _worker_thread = std::thread(&IpcClient::_worker, this);
}

void IpcClient::_worker() {
    // Poll items setup
    zmq_pollitem_t items[] = {{_socket, 0, ZMQ_POLLIN, 0}};

    QWISTYS_TODO_MSG("Need a proper way to kill a thread dua zmq_poll is indef");
    while (_poll) {
        // Poll for incoming messages
        int rc = zmq_poll(items, 1, -1);  // -1 means wait indefinitely

        if (rc == -1) {
            ERROR("zmq_poll");
            break;
        }

        // Check if there's incoming data on the dealer socket
        if (items[0].revents & ZMQ_POLLIN) {
            // Receive message
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_msg_recv(&msg, _socket, 0);

            // Process received message
            DEBUG("Received message from server: {}", (char*) zmq_msg_data(&msg));

            // Clean up message
            zmq_msg_close(&msg);
        }
    }
}

void IpcClient::send(const ipc_msg_t* msg) {
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(ipc_msg_t));
    memcpy(zmq_msg_data(&message), msg, sizeof(ipc_msg_t));
    zmq_send(_socket, "", 0, ZMQ_SNDMORE);
    zmq_msg_send(&message, _socket, 0);
    zmq_msg_close(&message);
}