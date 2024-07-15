#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include <cstddef>
#include <string>
#include <string_view>
#include "5thdbuffer.h"
#include "5thderror_handler.h"
#include "izmq.h"

/**
 * @brief Interface for the transmitter.
 * @note Currently we use ZMQ but need to check libp2p, also useful for
 * testing mocks.
 */
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
class ZMQWTransmitter final : public ITransmitter {
public:
    /**
     * @brief Destructor cleans all resources.
     */
    ~ZMQWTransmitter() override;

    /**
     * @brief Constructor
     */
    ZMQWTransmitter(ISocket* sock, std::string const& identity)
        : _error(_drp), _socket(sock), _identity(identity) {
        _init();
    };

    /**
     * @brief Connects to a target IP and port.
     * @param ip IP v4 address as string.
     * @param port Target port as int.
     * @note This method does not validate the IP address format.
     */
    bool connect(const std::string_view ip, int port) override;

    /**
    * @brief
    */
    bool disconnect(const std::string_view ip, int port) override;
    /**
     * @brief Closes the active connection.
     * @note This method will reset _socket to nullptr.
     */
    void close() override;

    /**
     * @brief Sends data through the active connection.
     * @param data Pointer to data.
     * @param num_bytes Size of the data in bytes.
     * @note Currently unimplemented.
     */
    bool send(const std::byte* data, size_t num_bytes) override;

    /**
     * @brief
     */
    void worker(std::atomic<bool>* until, const std::function<void(void*)>& callback) override;

    /**
     * @brief
     */
    int set_sockopt(int option_name, const void* option_value, size_t option_len) override;

    /**
     * @brief
     */
    void set_curve_client_options(const char* server_public_key);

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    ISocket* _socket;
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

#endif  // TRANSMITTER_H_
