#ifndef PEER_H
#define PEER_H

#include <unordered_set>
#include <vector>
#include "connection.h"
#include "izmq.h"
#include "receiver.h"

#define START_PORT 7099

int is_port_available(int port);

class Peer {
public:
    Peer(int port) : _port(port), _ctx_out(&_errors), _ipc_socket(nullptr) {
        init();
    };

    ~Peer();
    void connect(const std::string& ip, int port);
    void send(void* data, size_t data_length);
    int get_proto() { return _protocol->get_port(); };
    int get_selfport() { return _port; };
    void listen();

private:
    int _port;
    ZMQWContext _ctx_out;
    NetworkError _errors;
    std::vector<Connection> _connections;
    std::unique_ptr<Receiver> _receiver;
    std::unique_ptr<Receiver> _protocol;
    std::unordered_set<void*> _ipc_connections;
    void* _ipc_socket;
    
    void init();
    void _handle_new_connection(const std::string& ip, int port);
    void _connect(Connection& conn, const std::string& ip, int port, Connection::Transmitters client_type);
    void _setup_proto(Connection& conn);
    void _req_user_data(Connection& conn);
};

#endif  // PEER_H
