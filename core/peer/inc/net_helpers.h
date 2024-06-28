#ifndef NETWORK_HELPERS_H
#define NETWORK_HELPERS_H 

#include<string>

std::string sha256(const std::string& str);

int is_port_available(int port);

std::string get_local_ip();

std::string get_iface_name(const std::string& local_addr);

bool set_port_forward(int external_port, const std::string& internal_ip, int internal_port, std::string& iface);

#endif // NETWORK_HELPERS_H