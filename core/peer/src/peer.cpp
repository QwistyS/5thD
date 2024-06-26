#include <zmq.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "peer.h"
#include "qwistys_macro.h"

#include <arpa/inet.h>  //INET6_ADDRSTRLEN
#include <ifaddrs.h>
#include <miniupnpc.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <upnpcommands.h>

void get_ip() {
    char hostbuffer[256];
    struct hostent* host_entry;
    int hostname;
    struct in_addr** addr_list;

    // retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        family = ifa->ifa_addr->sa_family;

        // We only want IPv4 addresses
        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s != 0) {
                std::cerr << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
                exit(EXIT_FAILURE);
            }

            // Filter out localhost address
            std::string ip_address = host;
            if (ip_address.find("127.") != 0) {
                freeifaddrs(ifaddr);
                DEBUG("Internal ip {}", ip_address);
                return;
            }
        }
    }

    freeifaddrs(ifaddr);
    return;
}

std::string get_local_ip() {
    std::string localIP;
    struct ifaddrs *addrs, *tmp;

    if (getifaddrs(&addrs) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    tmp = addrs;
    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* pAddr = (struct sockaddr_in*) tmp->ifa_addr;
            localIP = inet_ntoa(pAddr->sin_addr);
            if (strcmp(tmp->ifa_name, "lo") != 0) {  // Exclude loopback interface
                break;
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
    return localIP;
}

int is_port_available(int port) {
    int _port = port;
    int ret = -1;
    sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        ERROR("FAIL TO OPEN SOCKET FOR PORT DISCOVERY");
        return ret;
    }

    while (true) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(_port);

        if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == Errors::OK) {
            ret = _port;
            break;
        } else {
            _port++;
            DEBUG("Checking next port {}", _port);
            if (_port > 7110) {
                return ret;
            }
        }
    }
    close(sock);
    DEBUG("Chosen socket port {}", _port);
    return _port;
}

bool set_port_forward(int external_port, const std::string& internal_ip, int internal_port) {
    struct UPNPDev* devlist;
    struct UPNPUrls urls;
    struct IGDdatas data;
    int error = 0;

    // Discover UPnP devices
    devlist = upnpDiscover(2000, NULL, NULL, 0, 0, 0, &error);

    if (devlist) {
        // Get the URLs and IGD data for UPnP device
        if (UPNP_GetValidIGD(devlist, &urls, &data, NULL, 0) == 1) {
            // Successfully found a UPnP enabled device (Internet Gateway Device)

            // Try to add a port mapping
            error =
                UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, std::to_string(external_port).c_str(),
                                    std::to_string(internal_port).c_str(), internal_ip.c_str(), NULL, "TCP", NULL, "0");

            if (error == UPNPCOMMAND_SUCCESS) {
                std::cout << "Successfully opened port " << external_port << " and forwarded to " << internal_ip << ":"
                          << internal_port << std::endl;
            } else {
                std::cerr << "Failed to open port. Error code: " << error << std::endl;
            }

            // Free URLs and data structures
            FreeUPNPUrls(&urls);
        } else {
            std::cerr << "No valid UPnP Internet Gateway Device found." << std::endl;
        }

        // Free discovered devices list
        freeUPNPDevlist(devlist);
    } else {
        std::cerr << "No UPnP devices found." << std::endl;
        return false;
    }

    return (error == UPNPCOMMAND_SUCCESS);
}

Peer::~Peer() {
    _connections.clear();
    _receiver->close();
    // _protocol->close();
    if (_ipc_socket) {
        zmq_close(_ipc_socket);
    }
}

void Peer::connect(const std::string& ip, int port) {
    QWISTYS_TODO_MSG("Make sure you test the performers vector vs list of <Connection>");
    _handle_new_connection(ip, port);
}

void Peer::send(void* data, size_t data_length) {
    DEBUG("Len of connection vector {}", _connections.size());
}

void Peer::listen() {
    if (_ipc_socket == nullptr) {
        ERROR("FAIL TO OPEN LISTENER");
        return;
    }
    _receiver = std::make_unique<Receiver>(_port, &_ctx_out, &_errors);

    auto _keys = Keys::get_instance();
    _receiver->set_curve_server_options(_keys->get_key(PRIVATE_KEY_FLAG), _keys->get_key(PUBLIC_KEY_FLAG));
    _receiver->listen();

    // auto next_port = is_port_available(_port + 1);

    // _protocol = std::make_unique<Receiver>(next_port, &_ctx_out, &_errors);
    // _protocol->set_curve_server_options(_keys->server_public_key, _keys->server_secret_key);
    // _protocol->listen();
}

void Peer::_connect(Connection& conn, const std::string& ip, int port, Connection::Transmitters client_type) {
    auto socket_type = 0;

    switch (client_type) {
        case Connection::USER:
            socket_type = ZMQ_REQ;
            conn.variant = Connection::USER;
            break;
        case Connection::PROTO:
            socket_type = ZMQ_REP;
            conn.variant = Connection::PROTO;
            break;
        default:
            break;
    }

    conn.clients.emplace_back(&_ctx_out, socket_type, &_errors);
    conn.clients[client_type].set_curve_client_options(Keys::get_instance()->get_key(PUBLIC_KEY_FLAG));
    conn.clients[client_type].connect(ip, port);

    switch (client_type) {
        case Connection::PROTO:
            _setup_proto(conn);
            break;
        default:
            break;
    }
}

void Peer::_setup_proto(Connection& conn) {
    QWISTYS_TODO_MSG("Handle setup proto");
    conn.trusted = true;
}

void Peer::_init() {
    get_ip();

    // Init Cache for connections
    _conn_cache = std::make_unique<LRU_Cache<std::string, Connection>>(CACHE_SIZE);

    // init socket
    _ipc_socket = zmq_socket(_ctx_out.get_context(), ZMQ_PAIR);
    if (_ipc_socket == nullptr) {
        ERROR("FAIL LISTEN");
        zmq_close(_ipc_socket);
        _errors.handle(Errors::FAIL_OPEN_SOCKET);
    }

    // init keys
    auto _keys = Keys::get_instance();

    QWISTYS_TODO_MSG("In init of the listener for a peer, do i need a way to configure sockopt? or all @ init?");
    // Set socket opt
    //
    // zmq_setsockopt(_ipc_socket, ZMQ_CURVE_SERVER, _keys->server_public_key, KEY_LENGTH - 1);

    // Bind
    if (zmq_bind(_ipc_socket, "inproc://workers") != Errors::OK) {
        ERROR("IPC Bind fail");
        zmq_close(_ipc_socket);
        _ipc_socket = nullptr;
        _errors.handle(Errors::FAIL_TO_BIND);
    }
}

void Peer::_handle_new_connection(const std::string& ip, int port) {
    QWISTYS_TODO_MSG("Add mutex for access to connections collection");
    // Create place for new connection
    _connections.emplace_back();
    auto& conn = _connections.back();
    // Connect to user
    _connect(conn, ip, port, Connection::USER);

    QWISTYS_TODO_MSG("Add retrieve the port for protocol")
    auto ret = conn.clients[Connection::USER].req_data(GENERIC_DATA);
    // _connect(ip, 5656, Connection::PROTO);

    QWISTYS_TODO_MSG("Handle when fail create connection")
    if (1) {
        DEBUG("Connection created successfully");
    } else {
        ERROR("Fail establish connection to {}:{}", ip, port);
        _connections.pop_back();
    }
}