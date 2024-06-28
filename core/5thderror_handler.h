#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include "5thderrors.h"


class IError {
public:
    virtual ~IError() = default;
    virtual void handle(const std::string& id, Errors e) = 0;
    virtual void reg(const std::string& id) = 0;
    virtual void unreg(const std::string& id) = 0;
    virtual Errors get_error(const std::string& id) = 0;
};

class NetworkError : public IError {
public:
    NetworkError();
    virtual ~NetworkError();
    void handle(const std::string& id, Errors e);
    void reg(const std::string& id);
    void unreg(const std::string& id);
    Errors get_error(const std::string& id);
    void dump();
private:
    std::unordered_map<std::string, Errors> _obj_status;
};

#endif  // ERROR_HANDLER_H