#include "keys_db.h"

std::string time_point_to_string(const std::chrono::system_clock::time_point& tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

VoidResult store_key(
    DatabaseAccess& db, const std::string& module_name, const std::string& key_type, const std::string& key_name,
    const std::vector<unsigned char>& key_value,
    const std::chrono::system_clock::time_point& expires_at = std::chrono::system_clock::time_point()) {
    std::string sql =
        "INSERT INTO module_keys (module_name, key_type, key_name, key_value, expires_at) VALUES (?, ?, ?, ?, ?)";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err(ErrorCode::FAIL_ADD_KEY, "Fail to add key");

    sqlite3_stmt* stmt_ptr = stmt.value();
    db.bind_blob(stmt_ptr, 1, std::vector<unsigned char>(module_name.begin(), module_name.end()));
    db.bind_blob(stmt_ptr, 2, std::vector<unsigned char>(key_type.begin(), key_type.end()));
    db.bind_blob(stmt_ptr, 3, std::vector<unsigned char>(key_name.begin(), key_name.end()));
    db.bind_blob(stmt_ptr, 4, key_value);

    if (expires_at != std::chrono::system_clock::time_point()) {
        std::string expires_at_str = time_point_to_string(expires_at);
        db.bind_blob(stmt_ptr, 5, std::vector<unsigned char>(expires_at_str.begin(), expires_at_str.end()));
    } else {
        sqlite3_bind_null(stmt_ptr, 5);
    }

    return db.execute(stmt_ptr);
}

VoidResult update_key(
    DatabaseAccess& db, const std::string& module_name, const std::string& key_type, const std::string& key_name,
    const std::vector<unsigned char>& new_key_value,
    const std::chrono::system_clock::time_point& new_expires_at = std::chrono::system_clock::time_point()) {
    std::string sql =
        "UPDATE module_keys SET key_value = ?, expires_at = ? WHERE module_name = ? AND key_type = ? AND key_name = ?";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err(ErrorCode::FAIL_UPDATE_KEY, "Fail to update key");

    auto stmt_ptr = stmt.value();
    db.bind_blob(stmt_ptr, 1, new_key_value);

    if (new_expires_at != std::chrono::system_clock::time_point()) {
        std::string expires_at_str = time_point_to_string(new_expires_at);
        db.bind_blob(stmt_ptr, 2, std::vector<unsigned char>(expires_at_str.begin(), expires_at_str.end()));
    } else {
        sqlite3_bind_null(stmt_ptr, 2);
    }

    db.bind_blob(stmt_ptr, 3, std::vector<unsigned char>(module_name.begin(), module_name.end()));
    db.bind_blob(stmt_ptr, 4, std::vector<unsigned char>(key_type.begin(), key_type.end()));
    db.bind_blob(stmt_ptr, 5, std::vector<unsigned char>(key_name.begin(), key_name.end()));

    return db.execute(stmt_ptr);
}

Result<std::vector<unsigned char>> get_key(DatabaseAccess& db, const std::string& module_name,
                                           const std::string& key_type, const std::string& key_name) {
    std::string sql =
        "SELECT key_value FROM module_keys WHERE module_name = ? AND key_type = ? AND key_name = ? AND is_active = 1";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err<std::vector<unsigned char>>(ErrorCode::FAIL_GET_KEY, "Fail to retrieve the key");

    auto stmt_ptr = stmt.value();
    db.bind_blob(stmt_ptr, 1, std::vector<unsigned char>(module_name.begin(), module_name.end()));
    db.bind_blob(stmt_ptr, 2, std::vector<unsigned char>(key_type.begin(), key_type.end()));
    db.bind_blob(stmt_ptr, 3, std::vector<unsigned char>(key_name.begin(), key_name.end()));

    sdbret_t result;
    auto query_result = db.qquery(sql, result);
    if (query_result.is_err() || result.rows.empty()) {
        return Err<std::vector<unsigned char>>(ErrorCode::KEY_NOT_FOUND, "Key not found");
    }

    return Ok<std::vector<unsigned char>>(result.rows[0].columns[0].second);
}

VoidResult remove_key(DatabaseAccess& db, const std::string& module_name, const std::string& key_type,
                      const std::string& key_name) {
    std::string sql = "UPDATE module_keys SET is_active = 0 WHERE module_name = ? AND key_type = ? AND key_name = ?";

    auto stmt = db.prepare(sql);
    if (stmt.is_err())
        return Err(ErrorCode::FAIL_REMOVE_KEY, "Fail to remove a key");

    auto stmt_ptr = stmt.value();
    db.bind_blob(stmt_ptr, 1, std::vector<unsigned char>(module_name.begin(), module_name.end()));
    db.bind_blob(stmt_ptr, 2, std::vector<unsigned char>(key_type.begin(), key_type.end()));
    db.bind_blob(stmt_ptr, 3, std::vector<unsigned char>(key_name.begin(), key_name.end()));

    return db.execute(stmt_ptr);
}