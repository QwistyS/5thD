#include "qwistys_macro.h"
#include "5thderror_handler.h"
#include "5thdlogger.h"


NetworkError::NetworkError() {
    DEBUG("CTOR Network Error Handler");
};

NetworkError::~NetworkError() {
    DEBUG("DECTOR Network Error Handler");
};

void NetworkError::handle(Errors e) {
    DEBUG("ERROR NETWORK {}", (int)e);
    QWISTYS_UNIMPLEMENTED();
}