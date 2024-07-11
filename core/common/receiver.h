#ifndef RECEIVER_H
#define RECEIVER_H

#include "izmq.h"

class IReceiver {
public:
    virtual ~IReceiver() = default;
    virtual bool listen() = 0;
    virtual void close() = 0;
    virtual int get_port() const = 0;
    virtual void worker(std::atomic<bool>* until, std::function<void(void*)> callback) = 0;
    virtual bool set_endpoint(const char* endpoint) = 0;
    virtual int set_sockopt(int option_name, const void* option_value, size_t option_len) = 0;
    virtual int get_sockopt(int option_name, void* option_value, size_t* option_len) = 0;
    virtual bool set_curve_server_options(const char* self_pub_key, const char* self_prv_key,
                                          size_t key_length_bytes) = 0;
};

class ZMQWReceiver : public IReceiver {
public:
    virtual ~ZMQWReceiver();
    ZMQWReceiver(std::string addr, int port, IContext* ctx, ISocket* socket)
        : _addr(addr), _port(port), _ctx(ctx), _socket(socket), _error(_drp) {
        _init();
    };
    bool listen() override;
    void close() override;
    void worker(std::atomic<bool>* until, std::function<void(void*)> callback) override;
    int get_port() const override { return _port; }
    bool set_curve_server_options(const char* self_pub_key, const char* self_prv_key, size_t key_length_bytes) override;
    bool set_endpoint(const char* endpoint) override;
    int set_sockopt(int option_name, const void* option_value, size_t option_len) override;
    int get_sockopt(int option_name, void* option_value, size_t* option_len) override;

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
    std::string _endpoint;
    // =============== Handle errors and recovery stuff
    VoidResult _listen();
    VoidResult _close();
    bool _handle_bind();
    void _setup_drp();
};

#endif  // RECEIVER_H