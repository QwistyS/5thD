#ifndef PEER_H
#define PEER_H

#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <zmq.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <spdlog/spdlog.h>
#include "peer_conn.h"

class Peer {
public:
    Peer();
    ~Peer();
    
    void connect(const std::string& ip, int port);
    void cleanupConnections();

private:
    std::list<Connection> connections;
    std::mutex connections_mutex;
    std::thread cleanup_thread;
    std::atomic<bool> running;
    void* zmq_ctx;
    
    SSL_CTX* ssl_ctx;

    void initializeSSL();
    void cleanupSSL();
    SSL* createSSL();

    void startCleanupThread();
    void stopCleanupThread();
};

#endif // PEER_H
