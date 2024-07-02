#ifndef PEER_H
#define PEER_H

#include <unordered_set>
#include <vector>
#include "5thdlru_cache.h"
#include "connection.h"
#include "izmq.h"
#include "receiver.h"
#include "transmitter.h"
#include "5thdtask.h"

#define START_PORT 7099
#define CACHE_SIZE 100
#define PEER_ID "PEER_ID"

typedef struct {
    conn_info_t conn_info;
    std::unordered_map<std::string, ZMQTransmitter> transmitters;
} client_t;

/**
 * @brief Peer main object
 * @note In future will be configured by configuration file or whatever
 * meanwhile STATIC DEFINES
 */
class Peer {
public:
    Peer(IError* e) : _ctx_out(e), _errors(e) { _init(); };

    ~Peer();
    void task(conn_info_t *info, Task task);

private:
    ZMQWContext _ctx_out;
    IError* _errors;
    std::vector<client_t> _connections;
    std::unique_ptr<Receiver> _receiver;
    std::unordered_set<void*> _ipc_connections;
    std::unique_ptr<LRU_Cache<std::string, client_t>> _conn_cache;
    std::string _self_id;
    void _init();

    void handle_listen(const conn_info_t* info);
    void handle_upnp(const conn_info_t* info);
    void handle_receiver_reinit(const conn_info_t* info);
    void init_receiver(const conn_info_t* info);
};

#endif  // PEER_H
