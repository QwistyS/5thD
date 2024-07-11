#ifndef MODULE_H
#define MODULE_H

#include <memory>
#include <vector>

#include "5thderror_handler.h"
#include "5thdipcmsg.h"
#include "keys_db.h"

struct KeysInfo{
    KeyType key_type;
    char* curve_pub;
    char* curve_prv;
    bool init();
    void deinit();
    bool is_ready;
};


struct Deleter {
    void operator()(void* ptr) const {}
};

typedef struct {
    std::vector<std::unique_ptr<void, Deleter>> unique_ptrs;
    Clients client_id;
    KeysInfo keys_info;

} module_init_t;

VoidResult module_init(module_init_t* config);
VoidResult module_deinit(module_init_t* config);

#endif  // MODULE_H