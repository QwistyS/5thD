#include <sodium.h>
#include <mutex>

#include "5thdallocator.h"
#include "5thderror_handler.h"
#include "5thdlogger.h"

std::mutex atomic_opt_mutex;

void* allocate_smem(size_t num_bytes) {
    if (sodium_init() == -1) {
        WARN("Sodium init error");
        return nullptr;
    }
    return sodium_malloc(num_bytes);
}

void free_smem(void* pdata, size_t num_bytes) {
    sodium_memzero(pdata, num_bytes);
    sodium_free(pdata);
}

void handle_atomic_opt(atomic_cb callback, void* data) {
    std::unique_lock<std::mutex> lock(atomic_opt_mutex);
    callback(data);
}

void init_sodium() {
    if (sodium_init() != (int) ErrorCode::OK) {
        ERROR("Fail to init sodium lib");
        std::abort();
    }
}