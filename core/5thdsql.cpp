#include "5thdsql.h"
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include "qwistys_macro.h"

#ifdef USE_EXPERIMENTAL_FILESYSTEM
#    include <experimental/filesystem>
#else
#    include <filesystem>
#endif

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
        return Err(ErrorCode::FAIL_OPEN_DB_FILE, "Fail to open db file: " + _db_path, Severity::HIGH);
    }

    rc = sqlite3_key(_db, _key, _key_num_byte);
    if (rc != SQLITE_OK) {
        ERROR("Failed decrypt. Error: {}", std::string(sqlite3_errmsg(_db)));
        sqlite3_close(_db);
        _db = nullptr;
        return Err(ErrorCode::FAIL_DECRYPT_DB, "fail to decrypt the database", Severity::HIGH);
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

VoidResult DatabaseAccess::begin_transaction() {
    return exec("BEGIN TRANSACTION;");
}

VoidResult DatabaseAccess::end_transaction() {
    return exec("COMMIT;");
}

VoidResult DatabaseAccess::close() {
    if (!_db) {
        DEBUG("Database already closed");
        return Ok();
    }

    // Finalize all prepared statements
    sqlite3_stmt* stmt;
    while ((stmt = sqlite3_next_stmt(_db, nullptr)) != nullptr) {
        sqlite3_finalize(stmt);
    }

    // Attempt to close the database
    int rc = sqlite3_close(_db);
    if (rc != SQLITE_OK) {
        std::string error_msg = sqlite3_errmsg(_db);
        ERROR("Failed to close database. Error code: {}. Error message: {}", rc, error_msg);

        // If there are still statements or unfinalized resources
        if (rc == SQLITE_BUSY) {
            ERROR("Database is busy. There might be unfinalized statements or unreleased resources.");

            // Attempt to get information about unfinalized statements
            sqlite3_stmt* stmt;
            while ((stmt = sqlite3_next_stmt(_db, nullptr)) != nullptr) {
                const char* sql = sqlite3_sql(stmt);
                ERROR("Unfinalized statement: {}", sql ? sql : "Unknown");
            }
        }

        return Err(ErrorCode::FAIL_CLOSE_DB, "Failed to close database: " + error_msg);
    }

    _db = nullptr;
    DEBUG("Database closed successfully");
    return Ok();
}

Result<sqlite3_stmt*> DatabaseAccess::prepare(const std::string& sql) {
    DEBUG("Preparing SQL: {}", sql);

    if (!_db) {
        ERROR("Database connection is null");
        return Err<sqlite3_stmt*>(ErrorCode::NO_VALID_DB, "Database connection is null");
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Failed to prepare statement. Error code: {}. Error message: {}", rc, sqlite3_errmsg(_db));
        return Err<sqlite3_stmt*>(ErrorCode::DB_PREPARE_STATEMENT_FAILED,
                                  "Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    DEBUG("Statement prepared successfully");
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

VoidResult DatabaseAccess::bind_int(sqlite3_stmt* stmt, int index, int value) {
    int rc = sqlite3_bind_int(stmt, index, value);
    if (rc != SQLITE_OK) {
        return Err(ErrorCode::DB_BIND_FAILED, "Failed to bind integer: " + std::string(sqlite3_errmsg(_db)));
    }
    return Ok();
}

VoidResult DatabaseAccess::bind_text(sqlite3_stmt* stmt, int index, const std::string& value) {
    int rc = sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        return Err(ErrorCode::DB_BIND_FAILED, "Failed to bind text: " + std::string(sqlite3_errmsg(_db)));
    }
    return Ok();
}

VoidResult DatabaseAccess::bind_null(sqlite3_stmt* stmt, int index) {
    int rc = sqlite3_bind_null(stmt, index);
    if (rc != SQLITE_OK) {
        return Err(ErrorCode::DB_BIND_FAILED, "Failed to bind null: " + std::string(sqlite3_errmsg(_db)));
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

VoidResult DatabaseAccess::add_scheme(const std::string& sql_script_path) {
    if (!_db) {
        return Err(ErrorCode::NO_VALID_DB, "Db pointer is null");
    }
    std::string schemaSQL = _read_scheme_script(sql_script_path);

    char* errmsg = nullptr;
    int rc = sqlite3_exec(_db, schemaSQL.c_str(), nullptr, nullptr, &errmsg);

    if (rc != SQLITE_OK) {
        std::string error_message = errmsg ? errmsg : "Unknown error";
        sqlite3_free(errmsg);
        ERROR("SQL error: {}", error_message);
        return Err(ErrorCode::FAIL_CREATE_DB_SCHEME, "Failed to create db scheme: " + error_message);
    }

    DEBUG("Schema applied successfully");
    return Ok();
}

VoidResult DatabaseAccess::verify_tables() {
    const char* check_tables_sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND (name='key_types' AND name='module_keys');";

    char* errmsg = nullptr;
    char** results = nullptr;
    int rows, columns;

    int rc = sqlite3_get_table(_db, check_tables_sql, &results, &rows, &columns, &errmsg);

    if (rc != SQLITE_OK) {
        std::string error_message = errmsg ? errmsg : "Unknown error";
        sqlite3_free(errmsg);
        ERROR("SQL error: {}", error_message);
        return Err(ErrorCode::DB_ERROR, "Failed to verify tables: " + error_message);
    }

    std::set<std::string> found_tables;
    for (int i = 1; i <= rows; i++) {  // Start from 1 to skip column names
        found_tables.insert(results[i * columns]);
    }

    sqlite3_free_table(results);

    if (found_tables.size() != 2) {
        ERROR("Missing tables. Found: {}", found_tables.size());
        for (const auto& table : found_tables) {
            DEBUG("Found table: {}", table);
        }
        return Err(ErrorCode::DB_MISSING_TABLES, "Not all required tables were created");
    }

    DEBUG("All required tables verified");
    return Ok();
}

VoidResult DatabaseAccess::query(sqlite3_stmt* stmt, sdbret_t& result) const {
    result.flush();

    int columnCount = sqlite3_column_count(stmt);
    int rc;

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
        ERROR("Db error {}", std::string(sqlite3_errmsg(_db)));
        return Err(ErrorCode::DB_ERROR, "Db step");
    }

    return Ok();
}

VoidResult DatabaseAccess::exec(const std::string& sql) {
    if (!_db) {
        return Err(ErrorCode::NO_VALID_DB, "Db pointer is null");
    }

    DEBUG("Executing SQL: {}", sql);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        ERROR("Failed to prepare statement: {} (Error code: {})", sqlite3_errmsg(_db), rc);
        return Err(ErrorCode::DB_PREPARE_STATEMENT_FAILED,
                   "Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // For PRAGMA statements, we might get here. Just continue.
    }

    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        ERROR("Failed to execute statement: {} (Error code: {})", sqlite3_errmsg(_db), rc);
        sqlite3_finalize(stmt);
        return Err(ErrorCode::DB_STATEMENT_EXECUTION_FAILED,
                   "Failed to execute statement: " + std::string(sqlite3_errmsg(_db)));
    }

    sqlite3_finalize(stmt);
    DEBUG("SQL executed successfully");
    return Ok();
}

std::string DatabaseAccess::_read_scheme_script(const std::string& sql_script_path) {
    std::ifstream file(sql_script_path);
    if (!file) {
        ERROR("Unable to open file: {}", sql_script_path);
        std::abort();
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void DatabaseAccess::_init(const std::string& path, const std::string encryption_key) {
    _db_path = path;
    DEBUG("Initializing database with path: {}", _db_path);

    _setup_drp();

    _key_num_byte = encryption_key.length();

    _key = (unsigned char*) sodium_malloc(_key_num_byte);
    if (_key == nullptr) {
        ERROR("Failed to allocate secure memory for key");
        std::abort();
    }
    memcpy(_key, encryption_key.c_str(), _key_num_byte);

    bool is_new_db = !std::filesystem::exists(_db_path);
    DEBUG("Database file exists: {}", is_new_db ? "No" : "Yes");

    if (is_new_db) {
        DEBUG("New database detected ... Initializing");
        auto ret = _new_db();
        if (ret.is_err()) {
            ERROR("Failed to create new database: {}", ret.error().message());
            if (!_error.handle_error(ret.error()))
                std::abort();
        }
    }
    open();
}

VoidResult DatabaseAccess::_new_db() {
    auto ret_open = open();
    if (ret_open.is_err()) {
        if (!_error.handle_error(ret_open.error()))
            std::abort();
    }

    // The path is meanwhile
    QWISTYS_TODO_MSG("create a proper links to path for sql scripts");
    std::string sql_script_path("/home/qwistys/src/5thD/db_scripts/table_keys.sql");
    if (!std::filesystem::exists(sql_script_path)) {
        ERROR("File sql script does not exist");
        std::abort();
    }
    auto ret_scheme = add_scheme(sql_script_path);
    if (ret_scheme.is_err()) {
        return Err(ErrorCode::FAIL_CREATE_DB_SCHEME, "Fail to create db scheme", Severity::HIGH);
    }

    auto verify_result = verify_tables();
    if (verify_result.is_err()) {
        ERROR("Failed to verify tables: {}", verify_result.error().message());
    }
    return Ok();
}

void DatabaseAccess::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::FAIT_INIT_DB, [this]() {
        WARN("Trying to recover init db");
        return _handle_new_db();
    });
    _drp.register_recovery_action(ErrorCode::FAIL_OPEN_DB_FILE, [this]() {
        WARN("Trying to recover open db");
        return _handle_open();
    });
    _drp.register_recovery_action(ErrorCode::FAIL_DECRYPT_DB, [this]() {
        WARN("Trying to recover decrypt db");
        return _handle_open();
    });
    _drp.register_recovery_action(ErrorCode::FAIL_CREATE_DB_SCHEME, [this]() {
        WARN("Trying to recover decrypt db");
        return _handle_add_scheme();
    });
}

bool DatabaseAccess::_handle_open() {
    return false;
}

bool DatabaseAccess::_handle_add_scheme() {
    return false;
}

bool DatabaseAccess::_handle_new_db() {
    return false;
}
