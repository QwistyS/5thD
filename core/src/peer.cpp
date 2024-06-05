#include "peer.h"
#include <iostream>
#include <algorithm>
#include <chrono>

Peer::Peer() 
    : running(true), zmq_ctx(zmq_ctx_new()), ssl_ctx(nullptr) {
    spdlog::info("Peer created, starting cleanup thread.");
    initializeSSL();
    startCleanupThread();
}

Peer::~Peer() {
    spdlog::info("Destroying peer, stopping cleanup thread.");
    stopCleanupThread();
    cleanupSSL();
    
}

void Peer::connect(const std::string& ip, int port) {
    Connection new_conn;
    
    // Create and configure the SSL connection
    new_conn.ssl = SSL_new(ssl_ctx);
    if (!new_conn.ssl) {
        spdlog::error("Failed to create SSL structure");
        ERR_print_errors_fp(stderr);
        return;
    }

    // Initialize the ZeroMQ socket
    new_conn.socket = zmq_socket(zmq_ctx, ZMQ_REQ);
    std::string address = "tcp://" + ip + ":" + std::to_string(port);

    if (zmq_connect(new_conn.socket, address.c_str()) == 0) {
        spdlog::info("Connected to {}", address);
        new_conn.connected = true;
    } else {
        spdlog::error("Failed to connect to {}", address);
        zmq_close(new_conn.socket);
        SSL_free(new_conn.ssl);
    }

    // Perform SSL handshake
    if (SSL_connect(new_conn.ssl) <= 0) {
        spdlog::error("SSL connection failed");
        unsigned long err;
        while ((err = ERR_get_error()) != 0) {
            char *str = ERR_error_string(err, NULL);
            spdlog::error("SSL error: {}", str);
        }
        SSL_free(new_conn.ssl);
        zmq_close(new_conn.socket);
        return;
    }
    std::lock_guard<std::mutex> lock(connections_mutex);
    connections.push_back(std::move(new_conn));
}

void Peer::cleanupConnections() {
    while (running) {
        {
            std::lock_guard<std::mutex> lock(connections_mutex);
            connections.remove_if([this](const Connection& conn) {
                // Check if connection is still valid
                if (!conn.connected) {
                    zmq_setsockopt(this->zmq_ctx, ZMQ_LINGER, "", 0);
                    zmq_close(conn.socket);
                    SSL_free(conn.ssl);
                    spdlog::info("Connection closed.");
                    return true;
                }
                return false;
            });
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Peer::startCleanupThread() {
    cleanup_thread = std::thread(&Peer::cleanupConnections, this);
}

void Peer::stopCleanupThread() {
    running = false;
    if (cleanup_thread.joinable()) {
        cleanup_thread.join();
    }
}

void Peer::initializeSSL() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!ssl_ctx) {
        spdlog::error("Failed to create SSL context");
        ERR_print_errors_fp(stderr);
    }
}

void Peer::cleanupSSL() {
    if (ssl_ctx) {
        SSL_CTX_free(ssl_ctx);
    }
    EVP_cleanup();
}

SSL* Peer::createSSL() {
    if (!ssl_ctx) return nullptr;

    SSL* ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        spdlog::error("Failed to create SSL object");
        ERR_print_errors_fp(stderr);
    }
    return ssl;
}
