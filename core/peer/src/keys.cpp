#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "5thderror_handler.h"
#include "5thdlogger.h"
#include "izmq.h"
#include "keys.h"

static Keys* instance = nullptr;

unsigned char MEANWHILE_KEY[44] = "We meant to be together, but we never meant";

bool create_file(const char* file) {
    FILE* fptr;
    bool ret = false;
    fptr = fopen(file, "w");
    if (fptr) {
        fclose(fptr);
        ret = true;
    }
    return ret;
}

bool is_path_exist(const char* path) {
    struct stat sb;
    return (stat(path, &sb) == 0);
}

int encrypt_file(const char* in_filename, const char* out_filename, const unsigned char* key) {
    FILE* in_file = fopen(in_filename, "rb");
    FILE* out_file = fopen(out_filename, "wb");
    if (!in_file || !out_file) {
        ERROR("File opening failed");
        return -1;
    }

    unsigned char iv[AES_BLOCK_SIZE];
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    fwrite(iv, 1, AES_BLOCK_SIZE, out_file);

    EVP_CIPHER_CTX* ctx;
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    unsigned char buffer[4096];
    unsigned char cipher_buffer[4096 + AES_BLOCK_SIZE];
    int bytes_read, cipher_bytes_written;
    int total_bytes_written = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_file)) > 0) {
        if (1 != EVP_EncryptUpdate(ctx, cipher_buffer, &cipher_bytes_written, buffer, bytes_read)) {
            ERROR("FAIL Rand bytes for iv");
            ERR_print_errors_fp(stderr);
            return -1;
        }
        fwrite(cipher_buffer, 1, cipher_bytes_written, out_file);
        total_bytes_written += cipher_bytes_written;
    }

    if (1 != EVP_EncryptFinal_ex(ctx, cipher_buffer, &cipher_bytes_written)) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    fwrite(cipher_buffer, 1, cipher_bytes_written, out_file);
    total_bytes_written += cipher_bytes_written;

    EVP_CIPHER_CTX_free(ctx);
    fclose(in_file);
    fclose(out_file);

    return total_bytes_written;
}

int decrypt_file(const char* in_filename, const char* out_filename, const unsigned char* key) {
    FILE* in_file = fopen(in_filename, "rb");
    FILE* out_file = fopen(out_filename, "wb");
    if (!in_file || !out_file) {
        perror("File opening failed");
        return -1;
    }

    unsigned char iv[AES_BLOCK_SIZE];
    fread(iv, 1, AES_BLOCK_SIZE, in_file);

    EVP_CIPHER_CTX* ctx;
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    unsigned char buffer[4096];
    unsigned char plain_buffer[4096 + AES_BLOCK_SIZE];
    int bytes_read, plain_bytes_written;
    int total_bytes_written = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_file)) > 0) {
        if (1 != EVP_DecryptUpdate(ctx, plain_buffer, &plain_bytes_written, buffer, bytes_read)) {
            ERROR("FAIL Rand bytes for iv");
            ERR_print_errors_fp(stderr);
            return -1;
        }
        fwrite(plain_buffer, 1, plain_bytes_written, out_file);
        total_bytes_written += plain_bytes_written;
    }

    if (1 != EVP_DecryptFinal_ex(ctx, plain_buffer, &plain_bytes_written)) {
        ERROR("FAIL Rand bytes for iv");
        ERR_print_errors_fp(stderr);
        return -1;
    }
    fwrite(plain_buffer, 1, plain_bytes_written, out_file);
    total_bytes_written += plain_bytes_written;

    EVP_CIPHER_CTX_free(ctx);
    fclose(in_file);
    fclose(out_file);

    return total_bytes_written;
}

void write_keys(const char* pub_key, const char* prv_key, const char* filename, const unsigned char* key) {
    FILE* file = fopen("temp.txt", "w");
    if (!file) {
        perror("File opening failed");
        return;
    }

    fprintf(file, "%s\n%s\n", pub_key, prv_key);
    fclose(file);

    encrypt_file("temp.txt", filename, key);
    remove("temp.txt");
}

void read_keys(char* pub_key, char* prv_key, const char* filename, const unsigned char* key) {
    decrypt_file(filename, "temp.txt", key);
    char buff[KEY_LENGTH + 1];
    int line = 0;
    FILE* file = fopen("temp.txt", "r");
    if (!file) {
        perror("File opening failed");
        return;
    }

    while (fgets(buff, sizeof(buff), file) != NULL) {
        (line == 0) ? memcpy(pub_key, buff, KEY_LENGTH) : memcpy(prv_key, buff, KEY_LENGTH);
        line++;
    }

    fclose(file);
    remove("temp.txt");
}

int keys_init(keys_t* k, int(gen_cb)(keys_t*)) {
    int ret = -1;

    if (k == nullptr) {
        ERROR("KEYS NOT INITED");
        return ret;
    }

    k->public_key = (char*) sodium_malloc(KEY_LENGTH);
    if (!k->public_key) {
        ERROR("SODIUM MALLOC FAIL");
        return ret;
    }

    k->private_key = (char*) sodium_malloc(KEY_LENGTH);
    if (!k->private_key) {
        sodium_free(k->private_key);
        ERROR("SODIUM MALLOC FAIL");
        return ret;
    }

    if (gen_cb) {
        ret = gen_cb(k);
    } else {
        ret = 0;
    }

    return ret;
}

void keys_deinit(keys_t* k) {
    sodium_free(k->public_key);
    sodium_free(k->private_key);
}

void keys_clear(keys_t* k) {
    sodium_memzero(k->public_key, KEY_LENGTH);
    sodium_memzero(k->private_key, KEY_LENGTH);
}

Keys::~Keys() {
    keys_deinit(&_keys);
}

Keys* Keys::get_instance() {
    if (instance == nullptr) {
        instance = new Keys(PATH_TO_KEY);
    }
    return instance;
}

const char* Keys::get_key(int flag) {
    switch (flag) {
        case PRIVATE_KEY_FLAG:
            return _keys.private_key;
        case PUBLIC_KEY_FLAG:
            return _keys.public_key;
        default:
            return nullptr;
    }
    return nullptr;
}

void Keys::_init(const char* path) {
    if (is_path_exist(path)) {
        if (keys_init(&_keys, nullptr) != Errors::OK) {
            ERROR("FAIL TO INIT KEYS");
            return;
        }
        read_keys(_keys.public_key, _keys.private_key, path, MEANWHILE_KEY);
    } else {
        if (create_file(path) != true) {
            ERROR("Fail to create file {}", path);
            return;
        }
        // generate new keys
        if (keys_init(&_keys, generate_keys) != Errors::OK) {
            ERROR("FAIL TO INIT KEYS");
        }
        write_keys(get_key(PUBLIC_KEY_FLAG), get_key(PRIVATE_KEY_FLAG), path, MEANWHILE_KEY);
    }
}
