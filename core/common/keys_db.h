#ifndef KEYS_DB_H
#define KEYS_DB_H

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include "5thdsql.h"
#include "5thderror_handler.h"

// Helper function to convert time_point to string
std::string time_point_to_string(const std::chrono::system_clock::time_point& tp);

/**
 * @brief
 */
VoidResult store_key(DatabaseAccess& db, const std::string& module_name, const std::string& key_type,
                     const std::string& key_name, const std::vector<unsigned char>& key_value,
                     const std::chrono::system_clock::time_point& expires_at);

/**
 * @brief
 */
VoidResult update_key(
    DatabaseAccess& db, const std::string& module_name, const std::string& key_type, const std::string& key_name,
    const std::vector<unsigned char>& new_key_value,
    const std::chrono::system_clock::time_point& new_expires_at);

/**
 * @brief
 */
Result<std::vector<unsigned char>> get_key(DatabaseAccess& db, const std::string& module_name,
                                           const std::string& key_type, const std::string& key_name);

/**
 * @brief
 */
VoidResult remove_key(DatabaseAccess& db, const std::string& module_name, const std::string& key_type,
                      const std::string& key_name);

#endif  // KEYS_DB_H