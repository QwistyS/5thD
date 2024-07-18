#ifndef SOFTWARE_BUS_H
#define SOFTWARE_BUS_H

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include "5thdbuffer.h"
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "5thdlogger.h"
#include "izmq.h"
#include "receiver.h"
#include "5thdtracy.h"


template <IReceiverConcept IReceiver>
class ZMQBus {
public:
    explicit ZMQBus(IReceiver& receiver) : _error(_drp), _router(receiver) { _init(); }
    ~ZMQBus();
    void set_security(const char* pub_key, const char* prv_key);
    void run();
    static void signal_handler(int sign);
    bool set_poll(bool state);

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    IReceiver& _router;
    std::unordered_map<int, std::string> _clients;
    std::mutex _clients_mutex;
    static std::atomic<bool> _poll;
    ManagedBuffer<ZMQAllMsg, 10> _msg_buffer = ManagedBuffer<ZMQAllMsg, 10>(init_allmsg, deinit_allmsg);
    void _init();
    void _handle_msg(void* sock);
    VoidResult _send_message(void* sock, const ipc_msg_t* msg, const std::string& identity);
};

template <IReceiverConcept IReceiver>
std::atomic<bool> ZMQBus<IReceiver>::_poll{true};

template <IReceiverConcept IReceiver>
bool ZMQBus<IReceiver>::set_poll(bool state) {
    _poll = state;
    DEBUG("Polling state changed to {}", state);
    return _poll;
}

template <IReceiverConcept IReceiver>
ZMQBus<IReceiver>::~ZMQBus() {
    _router.close();
    DEBUG("Closed bus");
}

template <IReceiverConcept IReceiver>
void ZMQBus<IReceiver>::set_security(const char* pub_key, const char* prv_key) {
    if (!_router.set_curve_server_options(pub_key, prv_key, 40)) {
        ERROR("Unsecure server ...");
    }
}

template <IReceiverConcept IReceiver>
void ZMQBus<IReceiver>::run() {
    _router.listen();
    _router.worker(&_poll, std::bind(&ZMQBus<IReceiver>::_handle_msg, this, std::placeholders::_1));
}

template <IReceiverConcept IReceiver>
void ZMQBus<IReceiver>::signal_handler(int sign) {
    if (sign == SIGINT || sign == SIGTERM) {
        DEBUG("Termination signal received. Cleaning up...");
        _poll.store(false);
    }
}

template <IReceiverConcept IReceiver>
void ZMQBus<IReceiver>::_init() {
    _router.set_endpoint(IPC_ENDPOINT);
}

template <IReceiverConcept IReceiver>
void ZMQBus<IReceiver>::_handle_msg(void* sock) {
    TELEMETRY("_handle_msg");
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

template <IReceiverConcept IReceiver>
VoidResult ZMQBus<IReceiver>::_send_message(void* sock, const ipc_msg_t* msg, const std::string& identity) {
    DEBUG("Message to {}", identity);
    return Ok();
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

#endif  // SOFTWARE_BUS_H
