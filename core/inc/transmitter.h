#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <string>
#include "context.h"

class ITransmmiter {
public:
    virtual ~ITransmmiter() = default;
    virtual void connect(const std::string& ip, int port) = 0;
    virtual void close() = 0;
    virtual void send(void* data) = 0;
    virtual bool is_connected() = 0;
};

class Transmiter : public ITransmmiter {
public:
    virtual ~Transmiter();
    Transmiter(IContext* ctx) : _ctx(ctx) {};
    void connect(const std::string& ip, int port);
    void close();
    void send(void* data);
    bool is_connected();
private:
    IContext* _ctx;
    void* _socket;
};

#endif  // TRANSMITTER_H