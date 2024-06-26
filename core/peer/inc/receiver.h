#ifndef RECEIVER_H
#define RECEIVER_H

#include <pthread.h>
#include "5thderror_handler.h"
#include "izmq.h"

static void* worker(void* args);

typedef struct {
    void* socket;
    bool active;
} thread_args_t;

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
    Receiver(int port, IContext* ctx, IError* error) : _ctx(ctx), _port(port), _error(error), _socket_rout(nullptr) {
        _init();
    };
    void listen() override;
    void close() override;
    int get_port() const override { return _port; }
    void set_curve_server_options(const char* server_public_key, const char* server_secret_key);

private:
    int _port;
    void* _socket_rout;
    IError* _error;
    IContext* _ctx;
    pthread_t _worker_thread;
    thread_args_t _thread_args;
    void _init();
};

#endif  // RECEIVER_H