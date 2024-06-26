#ifndef KEYS_H
#define KEYS_H

#define MEANWHILE_KEY "Sunshine, hope you having an awesome life."
#define PATH_KEYS "nothing_here.enc"
#define KEY_LENGTH 41

struct keys {
    char *server_public_key;
    char *server_secret_key;
};

void keys_init(keys* k);
void keys_deinit(keys* k);
void keys_clear(keys* k);

void write_keys(const char *server_public_key, const char *secret_key, const char *filename, const unsigned char *key);
void read_keys(char *server_public_key, char *secret_key, const char *filename, const unsigned char *key);


#endif // KEYS_H