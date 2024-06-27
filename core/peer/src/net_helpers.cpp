#include "net_helpers.h"
#include "5thdlogger.h"
#include "5thderrors.h"

#include <iostream>
#include <string>
#include <cstring>

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

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#else
#include <sys/types.h>
#endif


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

std::string get_local_ip() {
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    const char* googleDnsIp = "8.8.8.8";
    uint16_t dnsPort = 53;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
#if defined(_WIN32) || defined(_WIN64)
        WSACleanup();
#endif
        return "";
    }

    sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(googleDnsIp);
    serv.sin_port = htons(dnsPort);

    int err = connect(sock, (const sockaddr*)&serv, sizeof(serv));

    if (err < 0) {
#if defined(_WIN32) || defined(_WIN64)
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return "";
    }

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr*)&name, &namelen);

    if (err < 0) {
#if defined(_WIN32) || defined(_WIN64)
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return "";
    }

    char buffer[INET_ADDRSTRLEN];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
#if defined(_WIN32) || defined(_WIN64)
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return (p != nullptr) ? std::string(buffer) : "";
}

std::string get_iface_name(const std::string& local_addr) {
#if defined(_WIN32) || defined(_WIN64)
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_UNSPEC;
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    DWORD dwRetVal = 0;

    pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    if (pAddresses == nullptr) {
        std::cerr << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << std::endl;
        return "";
    }

    if ((dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen)) == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == nullptr) {
            std::cerr << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << std::endl;
            return "";
        }
    }

    if ((dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen)) == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next) {
                char buffer[INET_ADDRSTRLEN];
                DWORD bufferLength = INET_ADDRSTRLEN;
                WSAAddressToStringA(pUnicast->Address.lpSockaddr, (DWORD)pUnicast->Address.iSockaddrLength, nullptr, buffer, &bufferLength);
                if (ipAddress == buffer) {
                    std::wstring ws(pCurrAddresses->FriendlyName);
                    std::string interfaceName(ws.begin(), ws.end());
                    free(pAddresses);
                    return interfaceName;
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    } else {
        std::cerr << "Call to GetAdaptersAddresses failed with error: " << dwRetVal << std::endl;
    }

    if (pAddresses) {
        free(pAddresses);
    }
    return "";
#else
    struct ifaddrs* ifaddr, *ifa;
    int family;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return "";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr,
                                (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                      sizeof(struct sockaddr_in6),
                                host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s != 0) {
                std::cerr << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
                continue;
            }
            if (local_addr == host) {
                std::string interfaceName = ifa->ifa_name;
                freeifaddrs(ifaddr);
                return interfaceName;
            }
        }
    }

    freeifaddrs(ifaddr);
    return "";
#endif
}

bool set_port_forward(int external_port, const std::string& internal_ip, int internal_port, std::string& iface) {
    struct UPNPDev* devlist;
    struct UPNPUrls urls;
    struct IGDdatas data;
    int error = 0;

    // Discover UPnP devices
    devlist = upnpDiscover(2000, NULL, iface.c_str(), 0, 0, 0, &error);

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
