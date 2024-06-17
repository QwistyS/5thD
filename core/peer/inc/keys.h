#ifndef KEYS_H
#define KEYS_H

#define MEANWHILE_KEY "Sunshine, hope you having an awesome life."
#define PATH_KEYS "nothing_here.enc"
#define KEY_LENGTH 41

struct keys {
    char *public_key;
    char *secret_key;
};

void init(keys* k);
void deinit(keys* k);
void clear_keys(keys* k);

void write_keys(const char *public_key, const char *secret_key, const char *filename, const unsigned char *key);
void read_keys(char *public_key, char *secret_key, const char *filename, const unsigned char *key);


#endif // KEYS_H