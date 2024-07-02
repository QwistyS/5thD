#include "5thderrors.h"
#include "5thdlogger.h"
#include "net_helpers.h"
#include <curl/curl.h>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

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
#    include <iphlpapi.h>
#    include <winsock2.h>
#    include <ws2tcpip.h>
#    pragma comment(lib, "Ws2_32.lib")
#    pragma comment(lib, "Iphlpapi.lib")
#else
#    include <sys/types.h>
#endif

std::string sha256(const std::string& str) {
    unsigned char hash[EVP_MAX_MD_SIZE];  // Buffer to hold the hash
    unsigned int hash_len;                // Length of the resulting hash

    // Initialize OpenSSL's digest
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    // Initialize the context for SHA-256
    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("EVP_DigestInit_ex failed");
    }

    // Provide the data to be hashed
    if (1 != EVP_DigestUpdate(mdctx, str.c_str(), str.size())) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("EVP_DigestUpdate failed");
    }

    // Finalize the hash
    if (1 != EVP_DigestFinal_ex(mdctx, hash, &hash_len)) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("EVP_DigestFinal_ex failed");
    }

    // Clean up
    EVP_MD_CTX_free(mdctx);

    // Convert the hash to a hexadecimal string
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) hash[i];
    }
    return ss.str();
}

// Callback function to handle response data
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // Assuming response data is a null-terminated string
    std::string *response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string get_external_addr() {
    CURL *curl = curl_easy_init();
    std::string ip_address;

    if (curl) {
        // Set URL to fetch external IP
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");

        // Follow HTTP redirects if necessary
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Response data callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        // Response data container
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_address);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize libcurl." << std::endl;
    }

    return ip_address;
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

    int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));

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
    err = getsockname(sock, (sockaddr*) &name, &namelen);

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

    pAddresses = (IP_ADAPTER_ADDRESSES*) malloc(outBufLen);
    if (pAddresses == nullptr) {
        std::cerr << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << std::endl;
        return "";
    }

    if ((dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen)) == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES*) malloc(outBufLen);
        if (pAddresses == nullptr) {
            std::cerr << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << std::endl;
            return "";
        }
    }

    if ((dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen)) == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != nullptr;
                 pUnicast = pUnicast->Next) {
                char buffer[INET_ADDRSTRLEN];
                DWORD bufferLength = INET_ADDRSTRLEN;
                WSAAddressToStringA(pUnicast->Address.lpSockaddr, (DWORD) pUnicast->Address.iSockaddrLength, nullptr,
                                    buffer, &bufferLength);
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
    struct ifaddrs *ifaddr, *ifa;
    int family;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return "";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr,
                                (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host,
                                NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
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
    char lanaddr[64];
    int error = -1;

    // Discover UPnP devices
    devlist = upnpDiscover(2000, NULL, NULL, 0, 0, 2, NULL);
    if (devlist) {
        // Get the URLs and IGD data for UPnP device
        if (UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)) == 1) {
            // Successfully found a UPnP enabled device (Internet Gateway Device)

            // Try to add a port mapping
            error =
                UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, std::to_string(external_port).c_str(),
                                    std::to_string(internal_port).c_str(), internal_ip.c_str(), NULL, "TCP", NULL, "0");

            if (error == UPNPCOMMAND_SUCCESS) {
                DEBUG("Successfully opened port {} and forwarded to {}:{}", external_port, internal_ip, internal_port);
            } else {
                ERROR("Failed to open port. Error code: {}", error);
            }

            // Free URLs and data structures
            FreeUPNPUrls(&urls);
        } else {
            ERROR("No valid UPnP Internet Gateway Device found.");
        }

        // Free discovered devices list
        freeUPNPDevlist(devlist);
    } else {
        ERROR("No UPnP devices found.");
        return false;
    }

    return (error == UPNPCOMMAND_SUCCESS);
}
