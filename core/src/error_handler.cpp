#include "error_handler.h"
#include "qwistys_macro.h"
#include "lggr.h"


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