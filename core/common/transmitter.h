#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

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
    virtual void connect(const std::string& ip, int port) = 0;
    virtual void close() = 0;
    virtual void send(void* data, size_t data_length) const = 0;
    virtual void worker(std::atomic<bool>* until, std::function<void(void*)> callback) = 0;
    virtual int set_sockopt(int option_name, const void* option_value, size_t option_len) = 0;
};

/**
 * @brief Transmitter class handles all outgoing connections using
 * dependency injection.
 */
class ZMQTransmitter : public ITransmitter {
public:
    /**
     * @brief Destructor cleans all resources.
     */
    virtual ~ZMQTransmitter();

    /**
     * @brief Constructor
     */
    ZMQTransmitter(IContext* ctx, ISocket* sock, std::string identity)
        : _context(ctx), _socket(sock), _error(_drp), _identity(identity) {
        _init();
    };

    /**
     * @brief Connects to a target IP and port.
     * @param ip IP v4 address as string.
     * @param port Target port as int.
     * @note This method does not validate the IP address format.
     */
    void connect(const std::string& ip, int port) override;

    /**
     * @brief Closes the active connection.
     * @note This method will reset _socket to nullptr.
     */
    void close() override;

    /**
     * @brief Sends data through the active connection.
     * @param data Pointer to data.
     * @param data_length Size of the data in bytes.
     * @note Currently unimplemented.
     */
    void send(void* data, size_t data_length) const override;

    /**
     * @brief
     */
    void worker(std::atomic<bool>* until, std::function<void(void*)> callback) override;

    /**
     * @brief
     */
    int set_sockopt(int option_name, const void* option_value, size_t option_len) override;

    /**
     * @brief
     */
    void set_curve_client_options(const char* server_public_key);


protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    IContext* _context;
    ISocket* _socket;
    std::string _identity;
    void _init();
    VoidResult _connect(const std::string& ip, int port);
    bool _handle_connect();
};

#endif  // TRANSMITTER_H_
