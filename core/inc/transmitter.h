#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <string>
#include "context.h"
#include "error_handler.h"

class ITransmitter {
public:
    virtual ~ITransmitter() = default;
    virtual void connect(const std::string& ip, int port) = 0;
    virtual void close() = 0;
    virtual void send(void* data) = 0;
    virtual bool is_connected() = 0;
};

class Transmitter : public ITransmitter {
public:
    virtual ~Transmitter();
    Transmitter(IContext* ctx, IError* error) : _ctx(ctx), _error_handler(error) {};
    void connect(const std::string& ip, int port);
    void close();
    void send(void* data);
    bool is_connected();

private:
    IContext* _ctx;
    IError* _error_handler;
    void* _socket;
};

#endif  // TRANSMITTER_H