#include "izmq.h"
#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "5thdlogger.h"

void init_allmsg(ZMQAllMsg& msg) {
    int rc = 0;
    if (rc != (int) ErrorCode::OK) {
        WARN("Fail to init zmq message");
    }
    rc = zmq_msg_init_size(&msg.identity, strlen(CLIENTS_IDS[0]));
    if (rc != (int) ErrorCode::OK) {
        WARN("Fail to init zmq message");
    }
    rc = zmq_msg_init_size(&msg.empty, 0);

    if (rc != (int) ErrorCode::OK) {
        WARN("Fail to init zmq message");
    }
    rc = zmq_msg_init_size(&msg.msg, sizeof(ipc_msg_t));
}

void deinit_allmsg(ZMQAllMsg& msg) {
    if (zmq_msg_close(&msg.empty) != (int) ErrorCode::OK) {
        ERROR("Fail clean zmq msg");
    }
    if (zmq_msg_close(&msg.identity) != (int) ErrorCode::OK) {
        ERROR("Fail clean zmq msg");
    }
    if (zmq_msg_close(&msg.msg) != (int) ErrorCode::OK) {
        ERROR("Fail clean zmq msg");
    }
}

int generate_keys(char* public_key, char* private_key) {
    if (zmq_curve_keypair(public_key, private_key) != 0) {
        int err = zmq_errno();
        ERROR("Fail generate keys. Error from zmq: {}", zmq_strerror(err));
        return 1;
    }

    return 0;
}
