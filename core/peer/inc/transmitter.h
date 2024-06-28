#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include "5thderror_handler.h"
#include "izmq.h"
#include "connection.h"

#define GENERIC_DATA "Heartbeat"

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
    ZMQTransmitter(IContext* ctx, int socket_type, const std::string& id, IError* error)
        : _context(ctx), _socket_type(socket_type), _socket(nullptr), _error(error) {
        _init(id);
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
    void set_curve_client_options(const char* server_public_key);
    IContext* _context;
    IError* _error;
    void* _socket;
    int _socket_type;
    conn_info_t _self_info;

    void send_stream(void* data, size_t data_length, int chunk_size);
    void _init(const std::string& id);
    void _clear_buffers();
};

#endif  // TRANSMITTER_H_
