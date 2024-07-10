#include <errno.h>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "5thdlogger.h"
#include "software_bus.h"
#include "zmq.h"

std::atomic<bool> ZMQBus::_poll(true);  // Initialize as true

void init_allmsg(ZMQAllMsg& msg) {
    zmq_msg_init_size(&msg.identity, strlen(CLIENTS_IDS[0]));
    zmq_msg_init(&msg.empty);
    zmq_msg_init_size(&msg.msg, sizeof(ipc_msg_t));
}

void deinit_allmsg(ZMQAllMsg& msg) {
    zmq_msg_close(&msg.empty);
    zmq_msg_close(&msg.identity);
    zmq_msg_close(&msg.msg);
}

void print_ipc_msg(ipc_msg_t* msg) {
    printf("--- Frame ---\n");
    printf("src: %d\n", msg->src_id);
    printf("dist: %d\n", msg->dist_id);
    printf("timestamp: %ld\n", msg->timestamp);
    printf("category: %s\n", msg->category);
    printf("data: ");
    for (int i = 0; i < DATA_LENGTH_BYTES; i++) printf("%x", msg->data[i]);
    printf("\n");
}

VoidResult ZMQBus::_recv_message(void* sock, ipc_msg_t* msg) {
    auto request = _msg_buffer.get_slot();
    int rc;

    if (!request) {
        ERROR("Got null from bus buffer, Frame dropped");
        return Ok();
    }

    rc = zmq_msg_recv(&request->msg, sock, 0);
    if (rc == -1) {
        return Err(ErrorCode::FIAIL_RECV_MSG, "Failed to receive message");
    }

    if (zmq_msg_size(&request->msg) != sizeof(ipc_msg_t)) {
        return Err(ErrorCode::FIAIL_RECV_MSG, "Message size wierd :/");
    }

    memcpy(msg, zmq_msg_data(&request->msg), sizeof(ipc_msg_t));
    auto ret = _msg_buffer.release_slot(&request);
    if (ret.is_err()) {
        WARN("Buffer fail to release mem");
    }
    return Ok();
}

VoidResult ZMQBus::_send_message(void* sock, const ipc_msg_t* msg, const std::string& identity) {
    int rc;
    auto replay = _msg_buffer.get_slot();
    if (!replay) {
        ERROR("Got null from bus msg buffer frame dropped");
        return Ok();
    }

    memcpy(zmq_msg_data(&replay->identity), identity.c_str(), identity.size());
    rc = zmq_msg_send(&replay->identity, sock, ZMQ_SNDMORE);
    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Fail to send identity frame");
    }

    // Send empty delimiter frame
    rc = zmq_send(sock, "", 0, ZMQ_SNDMORE);
    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Fail to send empty frame");
    }

    memcpy(zmq_msg_data(&replay->msg), msg, sizeof(ipc_msg_t));
    rc = zmq_msg_send(&replay->msg, sock, 0);
    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Fail to send message frame");
    }

    auto ret = _msg_buffer.release_slot(&replay);
    if (ret.is_err()) {
        WARN("Buffer fail to release mem");
    }
    return Ok();
}

void ZMQBus::_handle_msg(void* sock) {
    int rc;
    auto all_msg = _msg_buffer.get_slot();
    ipc_msg_t data;

    if (!all_msg) {
        WARN("Dropping frame.");
        return;
    }

    rc = zmq_msg_recv(&all_msg->identity, sock, 0);
    if (rc == -1) {
        ERROR("Failed to receive identity: {}", zmq_strerror(zmq_errno()));
        return;
    }

    std::string src_id(static_cast<char*>(zmq_msg_data(&all_msg->identity)), zmq_msg_size(&all_msg->identity));

    rc = zmq_msg_recv(&all_msg->empty, sock, 0);
    if (rc == -1) {
        ERROR("Failed to receive delimiter: {}", zmq_strerror(zmq_errno()));
        return;
    }

    auto recv_ret = _recv_message(sock, &data);
    if (recv_ret.is_err()) {
        ERROR("Failed to receive message");
        _error.handle_error(recv_ret.error());
    }

    {
        std::lock_guard<std::mutex> lock(_clients_mutex);
        auto it_src = _clients.find(data.src_id);
        if (it_src == _clients.end()) {
            _clients.insert({data.src_id, src_id});
        }
    }

    DEBUG("Received from id: {}", src_id);
    // print_ipc_msg(&data);

    {
        std::lock_guard<std::mutex> lock(_clients_mutex);
        auto it_dst = _clients.find(data.dist_id);
        if (it_dst == _clients.end()) {
            WARN("The destination: {} never registered", CLIENTS_IDS[data.dist_id]);
            _msg_buffer.release_slot(&all_msg);
            return;
        }
        auto send_ret = _send_message(sock, &data, it_dst->second);
        if (send_ret.is_err()) {
            ERROR("Failed to send message");
            _error.handle_error(send_ret.error());
        }
    }
    _msg_buffer.release_slot(&all_msg);
}

ZMQBus::~ZMQBus() {
    _router->close();
}

void ZMQBus::set_security(const char* pub_key, const char* prv_key) {
    if (!pub_key) {
        WARN("NO Ecriptions set duo the fact the kye is null");
        std::abort();
    }
    int as_server = 1;
    int rc = _router->set_sockopt(ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
    if (rc != 0) {
        ERROR("Failed to set CURVE_SERVER option: {}", zmq_strerror(errno));
        std::abort();
    }

    rc = _router->set_sockopt(ZMQ_CURVE_PUBLICKEY, pub_key, 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_PUBLICKEY: {}", zmq_strerror(errno));
        std::abort();
    }

    rc = _router->set_sockopt(ZMQ_CURVE_SECRETKEY, prv_key, 40);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SECRETKEY: {}", zmq_strerror(errno));
        std::abort();
    }

    int has_curve;
    size_t has_curve_size = sizeof(has_curve);
    _router->get_sockopt(ZMQ_CURVE_SERVER, &has_curve, &has_curve_size);
    if (has_curve) {
        WARN("CURVE security is available");
    } else {
        ERROR("CURVE security is not available");
    }
}

void ZMQBus::run() {
    _router->listen();

    _router->worker(&_poll, std::bind(&ZMQBus::_handle_msg, this, std::placeholders::_1));
}

void ZMQBus::singal_handler(int sign) {
    if (sign == SIGINT || sign == SIGTERM) {
        DEBUG("Termination signal received. Cleaning up...");
        _poll = false;
    }
}

void ZMQBus::_init() {
    _router->set_endpoint(IPC_ENDPOINT);
}