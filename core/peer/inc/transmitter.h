#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include "5thderror_handler.h"
#include "izmq.h"

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
    virtual bool req_data(const char* OP) const = 0;
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
    ZMQTransmitter(IContext* ctx, int socket_type, IError* error)
        : _context(ctx), _socket_type(socket_type), _socket(nullptr), _error_handler(error) {
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
     * @brief Checks if the connection is alive.
     * @return True if connected, false otherwise.
     */
    bool req_data(const char* OP) const override;

private:
    IContext* _context;
    IError* _error_handler;
    void* _socket;
    int _socket_type;

    void send_stream(void* data, size_t data_length, int chunk_size);
    void _init();
    void _clear_buffers();
};

#endif  // TRANSMITTER_H_
