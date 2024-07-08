#include <fstream>
#include <sstream>
#include <stdexcept>
#include "5thdsql.h"

DatabaseAccess::~DatabaseAccess() {
    auto ret = close();
    if (ret.is_err()) {
        _error.handle_error(ret.error());
    }
    if (_key != nullptr) {
        sodium_free(_key);
        _key = nullptr;
    }
}

VoidResult DatabaseAccess::open() {
    if (_db != nullptr) {
        WARN("Database already open");
        return Ok();
    }

    int rc = sqlite3_open(_db_path.c_str(), &_db);
    if (rc != SQLITE_OK) {
        ERROR("Failed to open database. Error: {}", std::string(sqlite3_errmsg(_db)));
        sqlite3_close(_db);
        _db = nullptr;
        return Err(ErrorCode::FAIL_OPEN_DB_FILE, "Fail to open db file: " + _db_path);
    }

    rc = sqlite3_key(_db, _key, _key_num_byte);
    if (rc != SQLITE_OK) {
        ERROR("Failed decrypt. Error: {}", std::string(sqlite3_errmsg(_db)));
        sqlite3_close(_db);
        _db = nullptr;
        return Err(ErrorCode::FAIL_DECRYPT_DB, "fail to decrypt the database");
    }

    // Set WAL journal mode for better concurrency
    exec("PRAGMA journal_mode = WAL;");

    // Set busy timeout to handle lock contention
    sqlite3_busy_timeout(_db, 5000);  // 5 second timeout

    // If we've reached this point, the database was successfully opened and the key was set
    // Do i need to clear this key ?
    sodium_memzero(_key, _key_num_byte);
    sodium_free(_key);
    _key = nullptr;
    _key_num_byte = 0;
    return Ok();
}

VoidResult DatabaseAccess::close() {
    if (_db) {
        sqlite3_close(_db);
    }
    return Ok();
}

Result<sqlite3_stmt*> DatabaseAccess::prepare(const std::string& sql) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return Err<sqlite3_stmt*>(ErrorCode::DB_PREPARE_STATEMENT_FAILED,
                                  "Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }
    return Ok<sqlite3_stmt*>(std::move(stmt));
}

VoidResult DatabaseAccess::execute(sqlite3_stmt* stmt) {
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        return Err(ErrorCode::DB_STATEMENT_EXECUTION_FAILED,
                   "Failed to execute statement: " + std::string(sqlite3_errmsg(_db)));
    }
    return Ok();
}

VoidResult DatabaseAccess::bind_blob(sqlite3_stmt* stmt, int index, const std::vector<unsigned char>& data) {
    int rc = sqlite3_bind_blob(stmt, index, data.data(), data.size(), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        return Err(ErrorCode::DB_BIND_FAILED, "Failed to bind BLOB data: " + std::string(sqlite3_errmsg(_db)));
    }
    return Ok();
}

VoidResult DatabaseAccess::add_scheme(const std::string& scriptPath) {
    if (!_db) {
        return Err(ErrorCode::NO_VALID_DB, "Db pointer is null");
    }
    std::string schemaSQL = _read_scheme_script(scriptPath);
    auto ret = exec(schemaSQL);
    if (ret.is_err()) {
        _error.handle_error(ret.error());
    }

    return Ok();
}

VoidResult DatabaseAccess::qquery(const std::string& sql, sdbret_t& result) const {
    result.flush();

    if (!_db) {
        return Err(ErrorCode::NO_VALID_DB, "Db pointer is null");
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Db error {}", std::string(sqlite3_errmsg(_db)));
        return Err(ErrorCode::DB_ERROR, "Db prepare");
    }

    int columnCount = sqlite3_column_count(stmt);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        SecureQueryResult::Row row;
        for (int i = 0; i < columnCount; i++) {
            std::string columnName = sqlite3_column_name(stmt, i);
            int bytes = sqlite3_column_bytes(stmt, i);
            std::vector<unsigned char> value(bytes);

            if (sqlite3_column_type(stmt, i) != SQLITE_NULL) {
                const unsigned char* data = static_cast<const unsigned char*>(sqlite3_column_blob(stmt, i));
                std::copy(data, data + bytes, value.begin());
            }

            row.columns.emplace_back(columnName, std::move(value));
            result.totalSize += columnName.size() + bytes;
        }
        result.rows.push_back(std::move(row));
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        ERROR("Db error {}", std::string(sqlite3_errmsg(_db)));
        return Err(ErrorCode::DB_ERROR, "Db step");
    }

    sqlite3_finalize(stmt);
    return Ok();
}

VoidResult DatabaseAccess::exec(const std::string& sql) {
    if (!_db) {
        return Err(ErrorCode::NO_VALID_DB, "Db pointer is null");
    }

    char* errorMessage = nullptr;
    int rc = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        std::string error(errorMessage);
        sqlite3_free(errorMessage);
        ERROR("Db exec error {}", error);
        return Err(ErrorCode::FAIL_DB_EXEC, "Fail db exec");
    }
    return Ok();
}

std::string DatabaseAccess::_read_scheme_script(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        ERROR("Unable to open file: {}", filePath);
        std::abort();
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void DatabaseAccess::_init(const std::string& path, const std::string encryption_key) {
    _key_num_byte = encryption_key.length();
    _key = (unsigned char*) sodium_malloc(_key_num_byte);
    if (_key == nullptr) {
        throw std::runtime_error("Failed to allocate secure memory for key");
    }
    memcpy(_key, encryption_key.c_str(), _key_num_byte);

    bool is_new_db = !std::filesystem::exists(_db_path);

    if (is_new_db) {
        DEBUG("New data base detected ... Initializing");
        auto ret = _new_db();
        if (ret.is_err()) {
            _error.handle_error(ret.error());
        }
    }
}

VoidResult DatabaseAccess::_new_db() {
    auto ret_open = open();
    if (ret_open.is_err()) {
        _error.handle_error(ret_open.error());
    }

    auto ret_scheme = add_scheme("/Users/danielmor/src/5thD/db_scripts/table_keys.sql");
    if (ret_scheme.is_err()) {
        return Err(ErrorCode::FAIL_CREATE_DB_SCHEME, "Fail to create db scheme");
    }
    return Ok();
}

void DatabaseAccess::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::FAIT_INIT_DB, [this]() {
        WARN("Trying to recover init db");
        return _handle_new_db();
    });
}

bool DatabaseAccess::_handle_new_db() {
    if (1) {
        return false;
    }
    return true;
}
