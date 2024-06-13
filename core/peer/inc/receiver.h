#ifndef RECEIVER_H
#define RECEIVER_H

#include "5thderror_handler.h"
#include "izmq.h"

class IReceiver {
public:
    virtual ~IReceiver() = default;
    virtual void listen() = 0;
    virtual void close() = 0;
    virtual int get_port() const = 0;
};

class Receiver : public IReceiver {
public:
    virtual ~Receiver();
    Receiver(int port, IContext* ctx, IError* error)
        : _ctx(ctx), _port(port), _error(error), _socket_rout(nullptr) {
            init();
        };
    void listen() override;
    void close() override;
    int get_port() const override { return _port; }

private:
    int _port;
    void* _socket_rout;
    IError* _error;
    IContext* _ctx;

    void init();
};

#endif  // RECEIVER_H