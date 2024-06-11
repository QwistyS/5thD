#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "qwistys_macro.h"

NetworkError::NetworkError() {
    DEBUG("CTOR Network Error Handler");
};

NetworkError::~NetworkError() {
    DEBUG("DTOR Network Error Handler");
};

void NetworkError::handle(Errors e) {
    DEBUG("ERROR NETWORK {}", (int) e);
    QWISTYS_UNIMPLEMENTED();
}