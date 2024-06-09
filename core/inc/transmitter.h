#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <zmq.hpp>
#include <string>

#define CTX_TRANSMITTE 1

class ITransmmiter
{
public:
    virtual ~ITransmmiter() = default;
    virtual void connect(const std::string& ip, int port) = 0;
    virtual void close() = 0;
    virtual void send(void *data) = 0;
    virtual bool is_connected() = 0;
};

class Transmiter : public ITransmmiter
{
public:
    Transmiter() : _context(CTX_TRANSMITTE), _connected(false) {};
    // Move constructor
    public:
    Transmiter(zmq::context_t& context)
        : _context(CTX_TRANSMITTE), _connected(false) {}

    Transmiter(Transmiter&& other) noexcept
        : _context(CTX_TRANSMITTE),
          _socket(std::move(other._socket)),
          _connected(other._connected) {
        other._connected = false;
    }

    Transmiter& operator=(Transmiter&& other) noexcept {
        if (this != &other) {
           _context = std::move(other._context);
            _socket = std::move(other._socket);
            _connected = other._connected;
            other._connected = false;
            other._connected = false;
        }
        return *this;
    }

    // Delete the copy constructor and copy assignment operator
    Transmiter(const Transmiter&) = delete;
    Transmiter& operator=(const Transmiter&) = delete;

    virtual ~Transmiter();
    void connect(const std::string& ip, int port);
    void close();
    void send(void *data);
    bool is_connected();

private:
    zmq::context_t _context;
    std::unique_ptr<zmq::socket_t> _socket;
    bool _connected;
};

#endif // TRANSMITTER_H