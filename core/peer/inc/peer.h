#ifndef PEER_H
#define PEER_H

#include <vector>
#include "izmq.h"
#include "connection.h"

class Peer {
public:
    Peer(int port) : _ctx_out(&_errors) {};
    ~Peer();
    void connect(const std::string& ip, int port);
    void send(void* data, size_t data_length);

private:
    std::vector<Connection> _connections;
    ZMQWContext _ctx_out;
    NetworkError _errors;
    void _handle_new_connection(const std::string& ip, int port);
    void _connect(Connection& conn, const std::string& ip, int port, Connection::Transmitters client_type);
    void _setup_proto(Connection& conn);
    void _req_user_data(Connection& conn);
};

#endif  // PEER_H
