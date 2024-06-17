#include "keys.h"
#include <sodium.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"

void init(keys* k) {
    k->public_key = (char*) sodium_malloc(KEY_LENGTH);
    if (!k->public_key) {
        ERROR("SODIUM MALLOC FAIL");
    }
    k->secret_key = (char*) sodium_malloc(KEY_LENGTH);
    if (!k->secret_key) {
        ERROR("SODIUM MALLOC FAIL");
    }
}

void deinit(keys* k) {
    sodium_free(k->public_key);
    sodium_free(k->secret_key);
    clear_keys(k);
}

void clear_keys(keys* k) {
    sodium_memzero(k->public_key, KEY_LENGTH);
    sodium_memzero(k->secret_key, KEY_LENGTH);
}