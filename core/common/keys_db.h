#ifndef KEYS_DB_H
#define KEYS_DB_H

#include <chrono>
#include <string>
#include <vector>
#include "5thdsql.h"
#include "5thderror_handler.h"

// Debug stuff
VoidResult check_key_types(DatabaseAccess& db);

VoidResult print_schema(DatabaseAccess& db);

void print_all_keys(DatabaseAccess& db);


// Helper function to convert time_point to string
std::string time_point_to_string(const std::chrono::system_clock::time_point& tp);

// Enum for key types
enum class KeyType {
    CURVE25519,
    ED25519,
    AES,
    RSA,
    API_KEY
};

/**
 * @brief Initialize the key_types table
 */
VoidResult initialize_key_types(DatabaseAccess& db);

/**
 * @brief Store a new key
 */
VoidResult store_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type,
                     const std::string& key_name, const std::vector<unsigned char>& key_value,
                     const std::chrono::system_clock::time_point& expires_at = std::chrono::system_clock::time_point());

/**
 * @brief Update an existing key
 */
VoidResult update_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type,
                      const std::string& key_name, const std::vector<unsigned char>& new_key_value,
                      const std::chrono::system_clock::time_point& new_expires_at = std::chrono::system_clock::time_point());

/**
 * @brief Retrieve a key
 */
Result<std::vector<unsigned char>> get_key(DatabaseAccess& db, const std::string& module_name,
                                           KeyType key_type, const std::string& key_name);

/**
 * @brief Remove (deactivate) a key
 */
VoidResult remove_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type,
                      const std::string& key_name);

#endif  // KEYS_DB_H
