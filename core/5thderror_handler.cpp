#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

NetworkError::NetworkError() {
    DEBUG("CTOR Network Error Handler");
};

NetworkError::~NetworkError() {
    DEBUG("DTOR Network Error Handler");
}

void NetworkError::handle(const std::string& id, Errors e) {
    _obj_status[id] = e;
};

void NetworkError::reg(const std::string& id) {
    _obj_status[id] = Errors::OK;
}

void NetworkError::unreg(const std::string& id) {
    auto it = _obj_status.find(id);
    if (it != _obj_status.end()) {
        _obj_status.erase(it);
    }
}

Errors NetworkError::get_error(const std::string& id) {
    auto it = _obj_status.find(id);
    if (it != _obj_status.end()) {
        return it->second;
    } else {
        return Errors::NO_OBJECT;
    }
}

void NetworkError::dump() {
    for (auto& it : _obj_status) {
        DEBUG("ID {} STATUS {}", it.first, (int)it.second);
    }
}