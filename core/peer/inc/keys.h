#ifndef KEYS_H
#define KEYS_H

#include <sodium.h>

#define MEANWHILE_KEY "Sunshine, hope you having a awesome life."
#define PATH_KEYS "nothing_here.enc"

struct keys {
    char *public_key;
    char *secret_key;
};

void init(keys* k);
void deinit(keys* k);

void write_keys(const char *public_key, const char *secret_key, const char *filename, const unsigned char *key);
void read_keys(char *public_key, char *secret_key, const char *filename, const unsigned char *key);


#endif // KEYS_H