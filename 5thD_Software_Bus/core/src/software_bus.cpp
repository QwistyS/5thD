#include "zmq.h"

#include "software_bus.h"

std::atomic<bool> ZMQBus::_poll(true);  // Initialize as true

VoidResult ZMQBus::_recv_message(void* sock, ipc_msg_t* msg) {
    auto request = _msg_buffer.get_slot();
    int rc;

    if (!request) {
        ERROR("Got null from bus buffer, Frame dropped");
        return Ok();
    }

    rc = zmq_msg_recv(&request->msg, sock, 0);
    if (rc == -1) {
        return Err(ErrorCode::FAIL_RECV_MSG, "Failed to receive message");
    }

    if (zmq_msg_size(&request->msg) != sizeof(ipc_msg_t)) {
        return Err(ErrorCode::FAIL_RECV_MSG, "Message size weird :/");
    }

    memcpy(msg, zmq_msg_data(&request->msg), sizeof(ipc_msg_t));
    if (auto ret = _msg_buffer.release_slot(&request); ret.is_err()) {
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

    if (auto ret = _msg_buffer.release_slot(&replay); ret.is_err()) {
        WARN("Buffer fail to release mem");
    }

    return Ok();
}

void ZMQBus::_handle_msg(void* sock) {
    ipc_msg_t data;
    int rc;
    auto all_msg = _msg_buffer.get_slot();

    if (!all_msg) {
        ERROR("Fail retrieve slot from message buffer");
        return;
    }
    // RAII cleanup
    auto buffer_cleanup = [this](decltype(all_msg)* msg_slot) { _msg_buffer.release_slot(msg_slot); };
    std::unique_ptr<decltype(all_msg), decltype(buffer_cleanup)> all_msg_ptr(&all_msg, buffer_cleanup);

    // Receive identity frame
    rc = zmq_msg_recv(&all_msg->identity, sock, 0);
    if (rc == -1) {
        ERROR("Failed to receive identity: {}", zmq_strerror(zmq_errno()));
        return;
    }

    int64_t more = 1;
    size_t more_size = sizeof(more);
    do {
        zmq_msg_init(&all_msg->msg);
        rc = zmq_msg_recv(&all_msg->msg, sock, 0);
        if (rc == 0) {
            DEBUG("Empty frame received");
        } else {
            if (rc == sizeof(ipc_msg_t)) {
                std::memcpy(&data, zmq_msg_data(&all_msg->msg), rc);
            }
        }
        /* Determine if more message parts are to follow */
        rc = zmq_getsockopt(sock, ZMQ_RCVMORE, &more, &more_size);
        if (rc == -1) {
            WARN("Couldn't get socket option");
        }
        zmq_msg_close(&all_msg->msg);
    } while (more);

    // Copy sender Id
    std::string src_id(static_cast<char*>(zmq_msg_data(&all_msg->identity)), zmq_msg_size(&all_msg->identity));

    DEBUG("Received from id: {}", src_id);
    print_ipc_msg(&data);

    {
        std::scoped_lock<std::mutex> lock(_clients_mutex);
        if (auto it_src = _clients.find(data.src_id); it_src == _clients.end()) {
            _clients[data.src_id] = src_id;
        }

        auto it_dst = _clients.find(data.dist_id);
        if (it_dst == _clients.end()) {
            WARN("The destination: {} never registered", CLIENTS_IDS[data.dist_id]);
            return;
        }
        if (auto send_ret = _send_message(sock, &data, it_dst->second); send_ret.is_err()) {
            WARN("Fait send message Error {}", send_ret.error().message());
            _error.handle_error(send_ret.error());
        }
    }
}

ZMQBus::~ZMQBus() {
    _router->close();
    DEBUG("Closed bus");
}

void ZMQBus::set_security(const char* pub_key, const char* prv_key) {
    if (!_router->set_curve_server_options(pub_key, prv_key, 40)) {
        ERROR("Unsecure server ...");
    }
}

void ZMQBus::run() {
    _router->listen();

    _router->worker(&_poll, std::bind(&ZMQBus::_handle_msg, this, std::placeholders::_1));
}

void ZMQBus::signal_handler(int sign) {
    if (sign == SIGINT || sign == SIGTERM) {
        DEBUG("Termination signal received. Cleaning up...");
        _poll = false;
    }
}

void ZMQBus::_init() {
    _router->set_endpoint(IPC_ENDPOINT);
}
