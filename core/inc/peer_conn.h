#ifndef PEER_CONN_H
#define PEER_CONN_H

#include <zmq.h>
#include <openssl/ssl.h>
#include <string>

struct Connection {
    void* socket;
    SSL* ssl;
    bool connected;
    std::string uid;  // Unique identifier for the connection
    std::string peer_address;  // Address of the peer

    Connection() : socket(nullptr), ssl(nullptr), connected(false) {}
    
    // Function to close the connection and clean up resources
    void close() {
        if (socket) {
            zmq_close(socket);
            socket = nullptr;
        }
        if (ssl) {
            SSL_free(ssl);
            ssl = nullptr;
        }
        connected = false;
    }
};


#endif // PEER_CONN_H