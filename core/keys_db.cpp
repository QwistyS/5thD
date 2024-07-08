#include <iomanip>
#include <sstream>
#include "keys_db.h"

std::string time_point_to_string(const std::chrono::system_clock::time_point& tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

const char* key_type_to_string(KeyType key_type) {
    switch (key_type) {
        case KeyType::CURVE25519:
            return "CURVE25519";
        case KeyType::ED25519:
            return "ED25519";
        case KeyType::AES:
            return "AES";
        case KeyType::RSA:
            return "RSA";
        case KeyType::API_KEY:
            return "API_KEY";
        default:
            return "UNKNOWN";
    }
}

VoidResult store_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type, const std::string& key_name,
                     const std::vector<unsigned char>& key_value,
                     const std::chrono::system_clock::time_point& expires_at) {
    std::string sql = "INSERT INTO module_keys (module_name, key_type_id, key_name, key_value, expires_at) "
                      "VALUES (?, (SELECT id FROM key_types WHERE type_name = ?), ?, ?, ?)";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Failed to prepare statement to add key");

    sqlite3_stmt* stmt_ptr = stmt.value();

    auto bind_result = db.bind_text(stmt_ptr, 1, module_name);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Failed to bind module_name");

    const char* key_type_str = key_type_to_string(key_type);
    bind_result = db.bind_text(stmt_ptr, 2, key_type_str);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Failed to bind key_type");

    bind_result = db.bind_text(stmt_ptr, 3, key_name);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Failed to bind key_name");

    bind_result = db.bind_blob(stmt_ptr, 4, key_value);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Failed to bind key_value");

    if (expires_at != std::chrono::system_clock::time_point()) {
        std::string expires_at_str = time_point_to_string(expires_at);
        bind_result = db.bind_text(stmt_ptr, 5, expires_at_str);
    } else {
        bind_result = db.bind_null(stmt_ptr, 5);
    }
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Failed to bind expires_at");

    return db.execute(stmt_ptr);
}

VoidResult update_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type, const std::string& key_name,
                      const std::vector<unsigned char>& new_key_value,
                      const std::chrono::system_clock::time_point& new_expires_at) {
    std::string sql =
        "UPDATE module_keys SET key_value = ?, expires_at = ? "
        "WHERE module_name = ? AND key_type_id = (SELECT id FROM key_types WHERE type_name = ?) AND key_name = ?";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Failed to prepare statement to update key");

    auto stmt_ptr = stmt.value();

    auto bind_result = db.bind_blob(stmt_ptr, 1, new_key_value);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Failed to bind new_key_value");

    if (new_expires_at != std::chrono::system_clock::time_point()) {
        std::string expires_at_str = time_point_to_string(new_expires_at);
        bind_result = db.bind_text(stmt_ptr, 2, expires_at_str);
    } else {
        bind_result = db.bind_null(stmt_ptr, 2);
    }
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Failed to bind new_expires_at");

    bind_result = db.bind_text(stmt_ptr, 3, module_name);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Failed to bind module_name");

    const char* key_type_str = key_type_to_string(key_type);
    bind_result = db.bind_text(stmt_ptr, 4, key_type_str);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Failed to bind key_type");

    bind_result = db.bind_text(stmt_ptr, 5, key_name);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Failed to bind key_name");

    return db.execute(stmt_ptr);
}

Result<std::vector<unsigned char>> get_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type,
                                           const std::string& key_name) {
    std::string key_query = "SELECT key_value FROM module_keys "
                            "WHERE module_name = ? AND key_type_id = (SELECT id FROM key_types WHERE type_name = ?) "
                            "AND key_name = ? AND is_active = 1";
    auto key_stmt = db.prepare(key_query);
    if (key_stmt.is_err())
        return Err<std::vector<unsigned char>>(ErrorCode::FAIL_GET_KEY, "Failed to prepare key query");

    auto key_stmt_ptr = key_stmt.value();

    auto bind_result = db.bind_text(key_stmt_ptr, 1, module_name);
    if (bind_result.is_err())
        return Err<std::vector<unsigned char>>(ErrorCode::FAIL_GET_KEY, "Failed to bind module_name");

    const char* key_type_str = key_type_to_string(key_type);
    bind_result = db.bind_text(key_stmt_ptr, 2, key_type_str);
    if (bind_result.is_err())
        return Err<std::vector<unsigned char>>(ErrorCode::FAIL_GET_KEY, "Failed to bind key_type");

    bind_result = db.bind_text(key_stmt_ptr, 3, key_name);
    if (bind_result.is_err())
        return Err<std::vector<unsigned char>>(ErrorCode::FAIL_GET_KEY, "Failed to bind key_name");

    sdbret_t key_result;
    auto key_query_result = db.query(key_stmt_ptr, key_result);
    if (key_query_result.is_err() || key_result.rows.empty()) {
        return Err<std::vector<unsigned char>>(ErrorCode::KEY_NOT_FOUND, "Key not found");
    }

    return Ok<std::vector<unsigned char>>(key_result.rows[0].columns[0].second);
}

VoidResult remove_key(DatabaseAccess& db, const std::string& module_name, KeyType key_type,
                      const std::string& key_name) {
    std::string sql =
        "UPDATE module_keys SET is_active = 0 "
        "WHERE module_name = ? AND key_type_id = (SELECT id FROM key_types WHERE type_name = ?) AND key_name = ?";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err(ErrorCode::FAIL_REMOVE_KEY, "Failed to prepare statement to remove key");

    auto stmt_ptr = stmt.value();

    auto bind_result = db.bind_text(stmt_ptr, 1, module_name);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_REMOVE_KEY, "Failed to bind module_name");

    const char* key_type_str = key_type_to_string(key_type);
    bind_result = db.bind_text(stmt_ptr, 2, key_type_str);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_REMOVE_KEY, "Failed to bind key_type");

    bind_result = db.bind_text(stmt_ptr, 3, key_name);
    if (bind_result.is_err())
        return Err(ErrorCode::FAIL_REMOVE_KEY, "Failed to bind key_name");

    return db.execute(stmt_ptr);
}