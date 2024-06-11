#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "5thderror_handler.h"

/**
 * @brief Transmitter common interface for attached network library
 * context.
 * @note Currently we uze ZMQ but need to check libp2p, also useful for 
 * test mock
 */
class ITransmitterContext {
public:
    virtual ~ITransmitterContext() = default;
    virtual void* get_context() = 0;
};

/**
 * @brief Transmitter common interface for attached network library
 * context.
 * @note Currently we uze ZMQ but need to check libp2p, also useful for 
 * test mock
 */
class ITransmitter {
public:
    virtual ~ITransmitter() = default;
    virtual void connect(const std::string& ip, int port) = 0;
    virtual void close() = 0;
    virtual void send(void* data) = 0;
    virtual bool is_connected() = 0;
};

/**
 * @brief Transmitter class. Handles all outgoing connections
 * @note using dependency injection.
 */
class Transmitter : public ITransmitter {
public:
    /**
     * @brief DCTOR cleans all resources.
     */
    virtual ~Transmitter();
    /**
     * @brief CTOR
     */
    Transmitter(ITransmitterContext* ctx, IError* error) : _ctx(ctx), _error_handler(error) {};
    /**
     * @brief connecting to a target
     * @param ip IP v4 address as string
     * @param port target port as int
     * @note NOT checking if address provided as valid string 
     *  for example "1234/122.0.1" still considered as valid ip
     * 
     * @return void Error handled by Error handler
     */
    void connect(const std::string& ip, int port);
    /**
     * @brief Closing active connection
     * @note this method will reset _socket to nullptr
     * @return void Error handled by Error handler
     */
    void close();
    /**
     * @brief TBD
     * @note UNIMPLEMENTED
     */
    void send(void* data);
    /**
     * @brief checking if the connection is alive
     * @return bool true on success false on fail
     * @note Error handled by Error handler
     */
    bool is_connected();

private:
    ITransmitterContext* _ctx;
    IError* _error_handler;
    void* _socket;
};

#endif  // TRANSMITTER_H