#include "keys.h"

void init(keys* k) {
    k->public_key = (char*)sodium_malloc(KEY_LENGTH);
    k->secret_key = (char*)sodium_malloc(KEY_LENGTH);
}

void deinit(keys* k) {
    sodium_free(k->public_key);
    sodium_free(k->secret_key);
}

void clear(keys* k) {
    sodium_memzero(k->public_key, KEY_LENGTH);
    sodium_memzero(k->secret_key, KEY_LENGTH);
}