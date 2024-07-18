#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include <features.h>
#include <zmq.h>
#include <cstddef>
#include <string>
#include <string_view>

#include "5thdbuffer.h"
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "izmq.h"
#include "tracy/Tracy.hpp"

#define DEFAULT_DATA_CHUNK sizeof(ipc_msg_t)

/**
 * @brief Interface for the transmitter. consept c++20!!!
 * @note Currently we use ZMQ but need to check libp2p, also useful for
 * testing mocks.
 */
template <typename T>
concept ITransmiterConcept =
    requires(T t, std::atomic<bool>* until, std::function<void(void*)> callback, std::string_view ip, int port,
             std::byte* data, size_t num_bytes, int option_name, const void* option_value, size_t option_len) {
        { t.connect(ip, port) } -> std::same_as<bool>;
        { t.disconnect(ip, port) } -> std::same_as<bool>;
        { t.close() } -> std::same_as<void>;
        { t.send(data, num_bytes) } -> std::same_as<bool>;
        { t.worker(until, callback) } -> std::same_as<void>;
        { t.set_sockopt(option_name, option_value, option_len) } -> std::same_as<int>;
        { t.get_sockopt(option_name, option_value, option_len) } -> std::same_as<int>;
    };

class ITransmitter {
public:
    virtual ~ITransmitter() = default;
    virtual bool connect(const std::string_view ip, int port) = 0;
    virtual bool disconnect(const std::string_view ip, int port) = 0;
    virtual void close() = 0;
    virtual bool send(const std::byte* data, size_t num_bytes) = 0;
    virtual void worker(std::atomic<bool>* until, const std::function<void(void*)>& callback) = 0;
    virtual int set_sockopt(int option_name, const void* option_value, size_t option_len) = 0;
};

/**
 * @brief Transmitter class handles all outgoing connections using
 * dependency injection.
 */
template <SocketConcept Socket>
class ZMQWTransmitter final {
public:
    /**
     * @brief Destructor cleans all resources.
     */
    ~ZMQWTransmitter();

    /**
     * @brief Constructor
     */
    ZMQWTransmitter(Socket& sock, std::string_view identity) : _error(_drp), _socket(sock), _identity(identity) {
        _init();
    };

    /**
     * @brief Connects to a target IP and port.
     * @param ip IP v4 address as string.
     * @param port Target port as int.
     * @note This method does not validate the IP address format.
     */
    bool connect(const std::string_view ip, int port);

    /**
     * @brief
     */
    bool disconnect(const std::string_view ip, int port);
    /**
     * @brief Closes the active connection.
     * @note This method will reset _socket to nullptr.
     */
    void close();

    /**
     * @brief Sends data through the active connection.
     * @param data Pointer to data.
     * @param num_bytes Size of the data in bytes.
     * @note Currently unimplemented.
     */
    bool send(const std::byte* data, size_t num_bytes);

    /**
     * @brief
     */
    void worker(std::atomic<bool>* until, const std::function<void(void*)>& callback);

    /**
     * @brief
     */
    int set_sockopt(int option_name, const void* option_value, size_t option_len);

    /**
     * @brief
     */
    int get_sockopt(int option_name, const void* option_value, size_t option_len);

    /**
     * @brief
     */
    void set_curve_client_options(const char* server_public_key);

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    Socket& _socket;
    std::string _identity;
    ManagedBuffer<ZMQAllMsg, 10> _msg_buffer = ManagedBuffer<ZMQAllMsg, 10>(init_allmsg, deinit_allmsg);
    void _init();
    void _setup_drp();

    VoidResult _send(const std::byte* data, size_t num_bytes);
    VoidResult _connect(const std::string_view ip, int port);
    VoidResult _disconnect(std::string_view ip, int port);
    bool _handle_connect() const;
    bool _handle_msg_buff() const;
};

template <SocketConcept Socket>
ZMQWTransmitter<Socket>::~ZMQWTransmitter() {
    DEBUG("Closed transmitter");
}

template <SocketConcept Socket>
void ZMQWTransmitter<Socket>::_init() {
    zmq_setsockopt(_socket.get_socket(), ZMQ_IDENTITY, _identity.c_str(), _identity.size());

    _drp.register_recovery_action(ErrorCode::SOCKET_CONNECT_FAIL, [this]() { return _handle_connect(); });
}

template <SocketConcept Socket>
VoidResult ZMQWTransmitter<Socket>::_connect(const std::string_view ip, int port) {
    std::string endpoint;
    if (port == 0) {
        endpoint = ip;
    } else {
        endpoint = "tcp://" + std::string(ip) + ":" + std::to_string(port);
    }

    if (zmq_connect(_socket.get_socket(), endpoint.c_str()) != (int) ErrorCode::OK) {
        return Err(ErrorCode::SOCKET_CONNECT_FAIL, "Failed connect to " + endpoint);
    }

    // Poll the socket to check if it is connected
    zmq_pollitem_t poll_items[] = {{_socket.get_socket(), 0, ZMQ_POLLOUT, 0}};
    int timeout_ms = 5000;  // 5 seconds, or set your desired timeout value here

    if (int poll_rc = zmq_poll(poll_items, 1, timeout_ms); poll_rc == -1) {
        return Err(ErrorCode::FAIL_POLL_SOCKET, "Init Poll on connect to " + endpoint);

    } else if (poll_rc == 0) {
        return Err(ErrorCode::SOCKET_TIMEOUT, "Timeout waiting for srv " + endpoint);
    } else {
        if (poll_items[0].revents & ZMQ_POLLOUT) {
            DEBUG("Connected to {}", endpoint);
        } else {
            return Err(ErrorCode::MONKEY, "Unexpected stuff on poll event from zmq " + endpoint, Severity::HIGH);
        }
    }

    return Ok();
}

template <SocketConcept Socket>
VoidResult ZMQWTransmitter<Socket>::_disconnect(std::string_view ip, int port) {
    std::string endpoint;
    if (port == 0) {
        endpoint = std::string(ip);
    } else {
        endpoint = "tcp://" + std::string(ip) + ":" + std::to_string(port);
    }
    if (auto rc = zmq_disconnect(_socket.get_socket(), endpoint.c_str()); rc != (int) ErrorCode::OK) {
        Err(ErrorCode::SOCKET_DISCONNECT_FAIL, "Fail to disconnect from " + endpoint);
    }
    return Ok();
}

template <SocketConcept Socket>
void ZMQWTransmitter<Socket>::set_curve_client_options(const char* server_public_key) {
    QWISTYS_UNIMPLEMENTED();
}

template <SocketConcept Socket>
void ZMQWTransmitter<Socket>::worker(std::atomic<bool>* until, const std::function<void(void*)>& callback) {
    tracy::SetThreadName("Software buss thread");  // Set thread name for Tracy
    // Poll items setup
    zmq_pollitem_t items[] = {{_socket.get_socket(), 0, ZMQ_POLLIN, 0}};

    while (*until) {
        ZoneScopedNC("Software bus poll", 0x7edece);
        // Poll for incoming messages
        int rc = zmq_poll(items, 1, -1);  // -1 means wait indefinitely

        if (rc == -1) {
            ERROR("zmq_poll");
            break;
        }

        // Check if there's incoming data on the dealer socket
        if (items[0].revents & ZMQ_POLLIN) {
            callback(_socket.get_socket());
        }
    }
}

template <SocketConcept Socket>
int ZMQWTransmitter<Socket>::set_sockopt(int option_name, const void* option_value, size_t option_len) {
    return zmq_setsockopt(_socket.get_socket(), option_name, option_value, option_len);
}

template <SocketConcept Socket>
bool ZMQWTransmitter<Socket>::connect(const std::string_view ip, int port) {
    if (auto ret = _connect(ip, port); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

template <SocketConcept Socket>
bool ZMQWTransmitter<Socket>::disconnect(const std::string_view ip, int port) {
    if (auto ret = _disconnect(ip, port); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    DEBUG("Disconnected from {}:{}", ip, port);
    return true;
}

template <SocketConcept Socket>
void ZMQWTransmitter<Socket>::close() {
    WARN("If you need to close the transmitter object, then release/delete it");
}

template <SocketConcept Socket>
bool ZMQWTransmitter<Socket>::send(const std::byte* data, size_t data_length) {
    if (auto ret = _send(data, data_length); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

template <SocketConcept Socket>
VoidResult ZMQWTransmitter<Socket>::_send(const std::byte* data, size_t num_bytes) {
    int rc;
    size_t min = 0;

    if (_msg_buffer.is_full().value()) {
        return Err(ErrorCode::MANAGE_BUFF_FULL, "messages buffer is full", Severity::LOW);
    }
    auto all_msg = _msg_buffer.get_slot();

    // RAII cleanup
    auto buffer_cleanup = [this](decltype(all_msg)* msg_slot) { _msg_buffer.release_slot(msg_slot); };
    std::unique_ptr<decltype(all_msg), decltype(buffer_cleanup)> all_msg_ptr(&all_msg, buffer_cleanup);

    if (!all_msg) {
        WARN("Fail to allocate slot");
        return Err(ErrorCode::MANAGE_BUFF_MONKEY, "IDK what happen :/", Severity::MEDIUM);
    }

    // Ensure identity is valid
    if (_identity.empty()) {
        return Err(ErrorCode::INVALID_IDENTITY, "Identity is empty");
    }
    // Send identity
    memcpy(zmq_msg_data(&all_msg->identity), _identity.c_str(), _identity.size());
    rc = zmq_msg_send(&all_msg->identity, _socket.get_socket(), ZMQ_SNDMORE);

    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Failed to send identity frame");
    }

    // Send empty
    rc = zmq_msg_send(&all_msg->empty, _socket.get_socket(), ZMQ_SNDMORE);
    if (rc == -1) {
        return Err(ErrorCode::FAIL_SEND_FRAME, "Failed to send empty frame");
    }

    while (num_bytes > 0) {
        min = std::min(num_bytes, DEFAULT_DATA_CHUNK);

        zmq_msg_init_size(&all_msg->msg, min);
        memcpy(zmq_msg_data(&all_msg->msg), data, min);
        rc = zmq_msg_send(&all_msg->msg, _socket.get_socket(), (num_bytes > min) ? ZMQ_SNDMORE : 0);
        DEBUG("Sent {} bytes", rc);
        if (rc == -1) {
            zmq_msg_close(&all_msg->msg);
            return Err(ErrorCode::FAIL_SEND_FRAME, "Failed to send chunk of data");
        }
        zmq_msg_close(&all_msg->msg);

        data += min;
        num_bytes -= min;
    }
    return Ok();
}

template <SocketConcept Socket>
bool ZMQWTransmitter<Socket>::_handle_connect() const {
    WARN("Trying to resolve connect");
    return false;
}

template <SocketConcept Socket>
bool ZMQWTransmitter<Socket>::_handle_msg_buff() const {
    if (auto ret = _msg_buffer.check_integrity(); ret.is_err()) {
        return _error.handle_error(ret.error());
    }
    return true;
}

template <SocketConcept Socket>
void ZMQWTransmitter<Socket>::_setup_drp() {
    // clang-format off
    _drp.register_recovery_action(ErrorCode::SOCKET_CONNECT_FAIL, 
        [this]() { 
            return _handle_connect(); 
    });
    _drp.register_recovery_action(ErrorCode::MANAGE_BUFF_FULL, 
        [this]() { 
            return _handle_msg_buff();
    });
    // clang-format on
}

#endif  // TRANSMITTER_H_
