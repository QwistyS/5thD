#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include "5thderrors.h"


class IError {
public:
    virtual ~IError() = default;
    virtual void handle(Errors e) = 0;
};

class NetworkError : public IError {
public:
    NetworkError();
    virtual ~NetworkError();
    void handle(Errors e);
private:
};

#endif  // ERROR_HANDLER_H