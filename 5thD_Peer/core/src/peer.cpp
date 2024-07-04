#include "peer.h"

#include <zmq.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "net_helpers.h"
#include "qwistys_macro.h"

static int id_counter = 0;

Peer::~Peer() {
}

void Peer::task(conn_info_t* info, Task task) {
    switch (task) {
        case Task::TASK_DEFAULT:
            /* code */
            break;

        case Task::TASK_LISTEN:
            handle_listen(info);
            break;

        case Task::TASK_PORTFORWARD:
            handle_upnp(info);
            break;

        case Task::TASK_RECEIVER_REINIT:
            handle_receiver_reinit(info);
            break;

        default:
            break;
    }
}

void Peer::_init() {
    _self_id = PEER_ID + std::to_string(id_counter++);
    std::string ipc_id = _self_id + "ipc";
    _errors->reg(_self_id);
    _errors->handle(_self_id, Errors::OK);
    _ipc_client = std::make_unique<IpcClient>(_ctx_out.get_context(), ipc_id, _errors);
}

void Peer::handle_listen(const conn_info_t* info) {
    _errors->handle(_self_id, Errors::OK);

    _receiver->listen();
    auto err = _errors->get_error(_receiver->get_id());
    if (err != Errors::OK) {
        ERROR("Fail handle_listen receiver error {}", (int) err);
        _errors->handle(_self_id, err);
    }
}

void Peer::handle_upnp(const conn_info_t* info) {
    _errors->handle(_self_id, Errors::OK);

    auto iface = get_iface_name(info->addr);
    if (!set_port_forward(info->port, info->addr, info->port, iface)) {
        ERROR("Fail port forwarding {}//{}:{}", iface.c_str(), info->addr, (int) info->port);
        _errors->handle(_self_id, Errors::FAIL_UPNP_FORWARD);
    }
}

void Peer::handle_receiver_reinit(const conn_info_t* info) {
    if(_receiver)
        _receiver.reset();
        
    init_receiver(info);
}

void Peer::init_receiver(const conn_info_t* info) {
    // Init receiver
    _receiver = std::make_unique<Receiver>(info->addr, info->port, &_ctx_out, _errors);
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
