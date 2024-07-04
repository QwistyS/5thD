#include <errno.h>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "5thdlogger.h"
#include "software_bus.h"
#include "zmq.h"

void print_ipc_msg(ipc_msg_t* msg) {
    printf("--- Frame ---\n");
    printf("src: %d\n", msg->src_id);
    printf("dist: %d\n", msg->dist_id);
    printf("timestamp: %lld\n", msg->timestamp);
    printf("category: %s\n", msg->category);
    printf("data: ");
    for (int i = 0; i < DATA_LENGTH_BYTES; i++) printf("%x", msg->data[i]);
    printf("\n");
}

int recv_message(void* socket, ipc_msg_t* msg) {
    zmq_msg_t message;
    int rc = zmq_msg_init_size(&message, sizeof(ipc_msg_t));
    if (rc != 0) {
        ERROR("Failed to initialize message: {}", zmq_strerror(zmq_errno()));
        return -1;
    }

    rc = zmq_msg_recv(&message, socket, 0);
    if (rc == -1) {
        ERROR("Failed to receive message: {}", zmq_strerror(zmq_errno()));
        zmq_msg_close(&message);
        return -1;
    }

    if (zmq_msg_size(&message) != sizeof(ipc_msg_t)) {
        ERROR("Received message size mismatch. Expected {}, got {}", sizeof(ipc_msg_t), zmq_msg_size(&message));
        zmq_msg_close(&message);
        return -1;
    }

    memcpy(msg, zmq_msg_data(&message), sizeof(ipc_msg_t));
    zmq_msg_close(&message);
    return 0;
}

int send_message(void* socket, const ipc_msg_t* msg, const std::string& identity) {
    zmq_msg_t id;
    int rc = zmq_msg_init_size(&id, identity.size());
    if (rc != 0) {
        ERROR("Failed to initialize identity message: {}", zmq_strerror(zmq_errno()));
        return -1;
    }

    memcpy(zmq_msg_data(&id), identity.c_str(), identity.size());
    rc = zmq_msg_send(&id, socket, ZMQ_SNDMORE);
    if (rc == -1) {
        ERROR("Failed to send identity: {}", zmq_strerror(zmq_errno()));
        zmq_msg_close(&id);
        return -1;
    }
    zmq_msg_close(&id);

    // Send empty delimiter frame
    rc = zmq_send(socket, "", 0, ZMQ_SNDMORE);
    if (rc == -1) {
        ERROR("Failed to send delimiter: {}", zmq_strerror(zmq_errno()));
        return -1;
    }

    zmq_msg_t message;
    rc = zmq_msg_init_size(&message, sizeof(ipc_msg_t));
    if (rc != 0) {
        ERROR("Failed to initialize message: {}", zmq_strerror(zmq_errno()));
        return -1;
    }

    memcpy(zmq_msg_data(&message), msg, sizeof(ipc_msg_t));
    rc = zmq_msg_send(&message, socket, 0);
    if (rc == -1) {
        ERROR("Failed to send message: {}", zmq_strerror(zmq_errno()));
        zmq_msg_close(&message);
        return -1;
    }
    zmq_msg_close(&message);

    return 0;
}

void ZMQBus::_handle_msg(void* sock) {
    ipc_msg_t data;
    zmq_msg_t identity, empty;
    int rc;

    zmq_msg_init(&identity);
    rc = zmq_msg_recv(&identity, sock, 0);
    if (rc == -1) {
        ERROR("Failed to receive identity: {}", zmq_strerror(zmq_errno()));
        zmq_msg_close(&identity);
        return;
    }
    std::string src_id(static_cast<char*>(zmq_msg_data(&identity)), zmq_msg_size(&identity));
    zmq_msg_close(&identity);

    zmq_msg_init(&empty);
    rc = zmq_msg_recv(&empty, sock, 0);
    if (rc == -1) {
        ERROR("Failed to receive delimiter: {}", zmq_strerror(zmq_errno()));
        zmq_msg_close(&empty);
        return;
    }
    zmq_msg_close(&empty);

    rc = recv_message(sock, &data);
    if (rc == -1) {
        ERROR("Failed to receive message");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(_clients_mutex);
        auto it_src = _clients.find(data.src_id);
        if (it_src == _clients.end()) {
            _clients.insert({data.src_id, src_id});
        }
    }

    DEBUG("Received from id: {}", src_id);
    print_ipc_msg(&data);

    {
        std::lock_guard<std::mutex> lock(_clients_mutex);
        auto it_dst = _clients.find(data.dist_id);
        if (it_dst == _clients.end()) {
            WARN("The destination never registered");
            return;
        }
        rc = send_message(sock, &data, it_dst->second);
        if (rc == -1) {
            ERROR("Failed to send message");
        }
    }
}

ZMQBus::~ZMQBus() {
}

void ZMQBus::run() {
    _router->listen();

    _router->worker(&_poll, std::bind(&ZMQBus::_handle_msg, this, std::placeholders::_1));
}

void ZMQBus::_init() {
}
