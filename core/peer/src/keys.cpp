#include "keys.h"
#include <sodium.h>
#include "5thderror_handler.h"
#include "5thdlogger.h"

void keys_init(keys* k) {
    if (k == nullptr) {
        ERROR("KEYS NOT INITED");
        return;
    }
    k->server_public_key = (char*) sodium_malloc(KEY_LENGTH);
    if (!k->server_public_key) {
        ERROR("SODIUM MALLOC FAIL");
    }
    k->server_secret_key = (char*) sodium_malloc(KEY_LENGTH);
    if (!k->server_secret_key) {
        ERROR("SODIUM MALLOC FAIL");
    }
}

void keys_deinit(keys* k) {
    sodium_free(k->server_public_key);
    sodium_free(k->server_secret_key);
    keys_clear(k);
}

void keys_clear(keys* k) {
    sodium_memzero(k->server_public_key, KEY_LENGTH);
    sodium_memzero(k->server_secret_key, KEY_LENGTH);
}