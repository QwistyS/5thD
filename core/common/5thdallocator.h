#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <functional>

/**
 * @brief a callback for user to perform opt without interrupt
 * @param void* pointer to data for callback
 */
using atomic_cb = std::function<bool(void* data)>;

/**
 * @brief Wrapper for sodium malloc
 */
void* allocate_smem(size_t num_bytes);

/**
 * @brief Wrapper for sodium free
 */
void free_smem(void* pdata, size_t num_bytes);

/**
 * @brief Atomic callback call for user
 * @param atomic_cb the function you wanna to call
 * @param void* pointer to data
 * @note No check for null user should handle that its just a wrapper
 */
void handle_atomic_opt(const atomic_cb, void* data);

void init_sodium();

#endif  // ALLOCATOR_H