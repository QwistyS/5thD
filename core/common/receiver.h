#ifndef RECEIVER_H
#define RECEIVER_H

#include <atomic>
#include <functional>
#include <string>
#include "izmq.h"
#include "5thderror_handler.h"

template <typename T>
concept IReceiverConcept =
    requires(T t, std::atomic<bool>* until, std::function<void(void*)> callback, const char* endpoint, int option_name,
             const void* option_value, size_t option_len, void* option_value_out, size_t* option_len_out,
             const char* self_pub_key, const char* self_prv_key, size_t key_length_bytes) {
        { t.listen() } -> std::same_as<bool>;
        { t.close() } -> std::same_as<bool>;
        { t.get_port() } -> std::same_as<int>;
        { t.worker(until, callback) } -> std::same_as<void>;
        { t.set_endpoint(endpoint) } -> std::same_as<bool>;
        { t.set_sockopt(option_name, option_value, option_len) } -> std::same_as<int>;
        { t.get_sockopt(option_name, option_value_out, option_len_out) } -> std::same_as<int>;
        { t.set_curve_server_options(self_pub_key, self_prv_key, key_length_bytes) } -> std::same_as<bool>;
    };

template <SocketConcept Socket>
class ZMQWReceiver final {
public:
    ~ZMQWReceiver();
    ZMQWReceiver(const std::string_view addr, int port, Socket& socket)
        : _error(_drp), _port(port), _socket(socket), _addr(addr) {
        _init();
    }
    bool listen();
    bool close();
    void worker(std::atomic<bool>* until, std::function<void(void*)> callback);
    int get_port() const { return _port; }
    bool set_curve_server_options(const char* self_pub_key, const char* self_prv_key, size_t key_length_bytes);
    bool set_endpoint(const char* endpoint);
    int set_sockopt(int option_name, const void* option_value, size_t option_len);
    int get_sockopt(int option_name, void* option_value, size_t* option_len);

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    int _port;
    int _poll;
    std::string _addr;
    Socket& _socket;
    void _init();
    std::string _endpoint;
    // =============== Handle errors and recovery stuff
    VoidResult _listen();
    VoidResult _close();
    bool _handle_bind();
    void _setup_drp();
};

template <SocketConcept Socket>
ZMQWReceiver<Socket>::~ZMQWReceiver() {
    DEBUG("Closed receiver");
}

template <SocketConcept Socket>
void ZMQWReceiver<Socket>::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::FAIL_BIND_SOCKET, [this]() {
        WARN("Trying to recover error Receiver bind");
        return _handle_bind();
    });
}

template <SocketConcept Socket>
void ZMQWReceiver<Socket>::_init() {
    _setup_drp();
}

template <SocketConcept Socket>
void ZMQWReceiver<Socket>::worker(std::atomic<bool>* until, std::function<void(void*)> callback) {
    zmq_pollitem_t items[] = {{_socket.get_socket(), 0, ZMQ_POLLIN, 0}};

    DEBUG("Polling thread started");

    while (*until) {
        int ret = zmq_poll(items, 1, 500);  // Poll with 500 ms timeout

        if (ret == 0) {  // Timeout
            DEBUG("Timeout reached");
            continue;
        } else if (ret == -1) {
            ERROR("Error in zmq_poll {}", zmq_strerror(zmq_errno()));
            break;
        } else {
            if (items[0].revents & ZMQ_POLLIN) {
                DEBUG("Message received");
                if (callback == nullptr) {
                    DEBUG("No callback provided flushing Q");
                    zmq_msg_t msg;
                    int more = 0;
                    size_t more_size = sizeof(more);
                    do {
                        zmq_msg_init(&msg);
                        int rc = zmq_msg_recv(&msg, _socket.get_socket(), 0);
                        if (rc == -1) {
                            ERROR("Error receiving message: {}", zmq_strerror(zmq_errno()));
                            zmq_msg_close(&msg);
                            break;
                        }

                        DEBUG("Message part received: {}", zmq_msg_size(&msg));
                        zmq_msg_close(&msg);

                        // Check if more message parts are to follow
                        zmq_getsockopt(_socket.get_socket(), ZMQ_RCVMORE, &more, &more_size);
                    } while (more);

                    *until = false;  // Kill the poll
                } else {
                    callback(_socket.get_socket());
                }
            }
        }
    }
    DEBUG("Polling ended");
}

template <SocketConcept Socket>
VoidResult ZMQWReceiver<Socket>::_listen() {
    std::string endpoint;
    if (!_endpoint.empty()) {
        endpoint = _endpoint;
    } else {
        endpoint = "tcp://" + _addr + ":" + std::to_string(_port);
        _endpoint = endpoint;
    }

    if (zmq_bind(_socket.get_socket(), endpoint.c_str()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::FAIL_BIND_SOCKET, "Failed bind socket", Severity::HIGH);
    }

    DEBUG("Listener opened on {}", endpoint.c_str());

    return Ok();
}

template <SocketConcept Socket>
VoidResult ZMQWReceiver<Socket>::_close() {
    if (auto ret = zmq_unbind(_socket.get_socket(), _endpoint.c_str()); ret != (int) ErrorCode::OK) {
        return Err(ErrorCode::FAIL_BIND_SOCKET, "Fail to unbind on " + _endpoint);
    }
    return Ok();
}

template <SocketConcept Socket>
bool ZMQWReceiver<Socket>::_handle_bind() {
    WARN("Binding issue");
    WARN("Endpoint = {}", _endpoint.c_str());
    WARN("Address = {}", _addr.c_str());
    WARN("Port = {}", _port);

    return false;
}

template <SocketConcept Socket>
bool ZMQWReceiver<Socket>::listen() {
    if (auto ret = _listen(); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

template <SocketConcept Socket>
bool ZMQWReceiver<Socket>::set_curve_server_options(const char* self_pub_key, const char* self_prv_key,
                                            size_t key_length_bytes) {
    bool ret = false;

    if (!self_pub_key || !self_prv_key) {
        WARN("NO encryption set due to the fact the key is null");
        return ret;
    }

    int as_server = 1;
    int rc = zmq_setsockopt(_socket.get_socket(), ZMQ_CURVE_SERVER, &as_server, sizeof(as_server));
    if (rc != 0) {
        ERROR("Failed to set CURVE_SERVER option: {}", zmq_strerror(errno));
        return ret;
    }

    rc = zmq_setsockopt(_socket.get_socket(), ZMQ_CURVE_PUBLICKEY, self_pub_key, key_length_bytes);
    if (rc != 0) {
        ERROR("Failed to set CURVE_PUBLICKEY: {}", zmq_strerror(errno));
        return ret;
    }

    rc = zmq_setsockopt(_socket.get_socket(), ZMQ_CURVE_SECRETKEY, self_prv_key, key_length_bytes);
    if (rc != 0) {
        ERROR("Failed to set CURVE_SECRETKEY: {}", zmq_strerror(errno));
        return ret;
    }

    int has_curve;
    size_t has_curve_size = sizeof(has_curve);
    zmq_getsockopt(_socket.get_socket(), ZMQ_CURVE_SERVER, &has_curve, &has_curve_size);

    if (has_curve) {
        DEBUG("CURVE security is available");
        ret = true;
    } else {
        WARN("CURVE security is not available");
    }

    return ret;
}

template <SocketConcept Socket>
bool ZMQWReceiver<Socket>::set_endpoint(const char* endpoint) {
    if (!endpoint) {
        return false;
    }
    _endpoint = endpoint;
    return !_endpoint.empty();
}

template <SocketConcept Socket>
int ZMQWReceiver<Socket>::set_sockopt(int option_name, const void* option_value, size_t option_len) {
    return zmq_setsockopt(_socket.get_socket(), option_name, option_value, option_len);
}

template <SocketConcept Socket>
int ZMQWReceiver<Socket>::get_sockopt(int option_name, void* option_value, size_t* option_len) {
    return zmq_getsockopt(_socket.get_socket(), option_name, option_value, option_len);
}

template <SocketConcept Socket>
bool ZMQWReceiver<Socket>::close() {
    if (auto ret = _close(); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    DEBUG("Unbind on {}", _endpoint);
    return true;
}

#endif  // RECEIVER_H
