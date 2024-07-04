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
    virtual void worker(std::atomic<bool>* until, std::function<void(void*)> callback) = 0;
};

class ZMQReceiver : public IReceiver {
public:
    virtual ~ZMQReceiver();
    ZMQReceiver(std::string addr, int port, IContext* ctx, ISocket* socket)
        : _addr(addr), _port(port), _ctx(ctx), _socket(socket), _error(_drp) {
        _init();
    };
    void listen() override;
    void close() override;
    void worker(std::atomic<bool>* until, std::function<void(void*)> callback) override;
    int get_port() const override { return _port; }
    void set_curve_server_options(const char* server_public_key, const char* server_secret_key,
                                  size_t key_length_bytes);

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    int _port;
    int _poll;
    IContext* _ctx;
    ISocket* _socket;
    std::string _addr;
    void _init();
    // =============== Handle errors and recovery stuff
    VoidResult _listen();
    VoidResult _close();
    bool _handle_bind_issue();
    void _setup_drp();
};

#endif  // RECEIVER_H