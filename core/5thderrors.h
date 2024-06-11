#ifndef ERRORS_H
#define ERRORS_H

enum Errors {
    OK = 0,
    SOCKET_INIT_FAIL,
    SOCKET_CLOSE_FAIL,
    SEND_HEARTBEAT_FAIL,
    FAIL_CLOSE_ZQM_CTX,
    FAIL_OPEN_ZMQ_CTX,
    MONKEY,
    TOTAL
};

#endif // ERRORS_H