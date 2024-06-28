#include "izmq.h"
#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "keys.h"
#include <zmq.h>

static int id_counter = 0;

int generate_keys(keys_t* keys) {
    return zmq_curve_keypair(keys->public_key, keys->private_key);
}

ZMQWContext::~ZMQWContext() {
    if (_context)
        _close();
        _error->unreg(_self_id);
        id_counter--;
}

void* ZMQWContext::get_context() {
    return _context;
}

void ZMQWContext::_init() {
    _self_id = ZMQWRAPPER_ID + std::to_string(id_counter++);
    _error->reg(_self_id);
    _set_context();
}

void ZMQWContext::_close() {
    _error->handle(_self_id, Errors::OK);

    if (_context) {
        if (zmq_ctx_destroy(_context) != Errors::OK) {
            ERROR("ZMQ FAIL clear ctx");
            _error->handle(_self_id, Errors::FAIL_CLOSE_ZQM_CTX);
        } else {
            DEBUG("ZMQ clear ctx success");
        }
    }
    _context = nullptr;
}

void ZMQWContext::_set_context() {
    _error->handle(_self_id, Errors::OK);
    _context = zmq_ctx_new();
    if (_context) {
        zmq_ctx_set(_context, ZMQ_IO_THREADS, 2);
        DEBUG("ZMQ Context created success");
    } else {
        _context = nullptr;
        ERROR("ZMQ Context fail to create");
        _error->handle(_self_id, Errors::FAIL_OPEN_ZMQ_CTX);
    }
}