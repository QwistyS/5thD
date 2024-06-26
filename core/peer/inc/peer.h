#ifndef PEER_H
#define PEER_H

#include <unordered_set>
#include <vector>
#include "connection.h"
#include "izmq.h"
#include "receiver.h"
#include "5thdlru_cache.h"

#define START_PORT 7099
#define CACHE_SIZE 100

std::string get_local_ip();

int is_port_available(int port);

bool set_port_forward(int external_port, const std::string& internal_ip, int internal_port);

/**
 * @brief Peer main object
 * @note In future will be configured by configuration file or whatever
 * meanwhile STATIC DEFINES
 */
class Peer {
public:
    Peer(int port) : 
        _port(port), _ctx_out(&_errors), _ipc_socket(nullptr) { _init(); };

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
    std::unique_ptr<LRU_Cache<std::string, Connection>> _conn_cache;
    void* _ipc_socket;

    void _init();
    void _handle_new_connection(const std::string& ip, int port);
    void _connect(Connection& conn, const std::string& ip, int port, Connection::Transmitters client_type);
    void _setup_proto(Connection& conn);
    void _req_user_data(Connection& conn);
};

#endif  // PEER_H
