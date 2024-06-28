#include <zmq.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "peer.h"
#include "qwistys_macro.h"

static int id_counter = 0;

Peer::~Peer() {
}

void Peer::task(conn_info_t info, Task task) {

    switch (task)
    {
    case Task::TASK_DEFAULT:
        /* code */
        break;

    case Task::TASK_LISTEN:
        handle_listen(info);
        break;
    
    default:
        break;
    }

}

void Peer::_init() {
    _self_id = PEER_ID + std::to_string(id_counter++);
    _errors->reg(_self_id);
    _errors->handle(_self_id, Errors::OK);
    // Init receiver
    _receiver = std::make_unique<Receiver>(_port, &_ctx_out, _errors);
    if (_receiver == nullptr) {
        ERROR("Fail to init receiver");
        _errors->handle(_self_id, Errors::OBJ_RECEIVER_INIT_FAIL);
    }
    auto err = _errors->get_error(_receiver->get_id());
    if (err != Errors::OK) {
        ERROR("Receiver inited with error {}", (int) err);
        _errors->handle(_self_id, Errors::OBJ_RECEIVER_INIT_FAIL);
    }
}

void Peer::handle_listen(conn_info_t info) {
    _errors->handle(_self_id, Errors::OK);

    _receiver->listen();
    auto err = _errors->get_error(_receiver->get_id());
    if(err != Errors::OK) {
        ERROR("Fail handle_listen receiver error {}", (int)err);
        _errors->handle(_self_id, err);
    }
}
