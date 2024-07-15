#ifndef SQL_H
#define SQL_H

#include <string_view>
#include <sodium.h>
// Include the SQLite header with SQLITE_HAS_CODEC defined
#define SQLITE_HAS_CODEC 1
#include <sqlcipher/sqlite3.h>
#include <memory>
#include <string>
#include <vector>

#include "5thderror_handler.h"

#ifndef DB_PATH
#    define DB_PATH "/home/qwistys/src/5thD/bin/fithd.db"
#endif

inline constexpr char MEANWHILE_DB_KEY[] = "It was meant to be but not meant to last";

using sdbret_t = struct SecureQueryResult {
    struct Row {
        std::vector<std::pair<std::string, std::vector<unsigned char>>> columns;
    };
    std::vector<Row> rows;
    size_t totalSize;

    SecureQueryResult() : totalSize(0) {}

    ~SecureQueryResult() { flush(); }

    void flush() {
        for (auto const& row : rows) {
            for (auto [first, second] : row.columns) {
                sodium_memzero(second.data(), second.size());
            }
        }
        rows.clear();
        totalSize = 0;
    }
};

class DatabaseAccess {
public:
    DatabaseAccess(const std::string_view path = DB_PATH, const std::string_view encryption_key = MEANWHILE_DB_KEY)
        : _error(_drp), _db_path(path) {
        _init(path, encryption_key);
    }
    ~DatabaseAccess();
    DatabaseAccess(const DatabaseAccess&) = delete;
    DatabaseAccess& operator=(const DatabaseAccess&) = delete;

    /**
     * @brief
     */
    VoidResult open();

    /**
     * @brief
     */
    VoidResult begin_transaction();

    /**
     * @brief
     */
    VoidResult end_transaction();
    /**
     * @brief
     */
    VoidResult close();

    /**
     * @brief
     */
    VoidResult exec(const std::string& sql);

    /**
     * @brief
     */
    VoidResult add_scheme(const std::string& sql_script_path);

    /**
     * @brief
     */
    VoidResult query(sqlite3_stmt* stmt, sdbret_t& result) const;

    /**
     * @brief
     */
    VoidResult bind_text(sqlite3_stmt* stmt, int index, const std::string& value);

    /**
     * @brief
     */
    VoidResult bind_null(sqlite3_stmt* stmt, int index);

    /**
     * @brief
     */
    VoidResult bind_int(sqlite3_stmt* stmt, int index, int value);
    /**
     * @brief
     */
    VoidResult bind_blob(sqlite3_stmt* stmt, int index, const std::vector<unsigned char>& data);

    /**
     * @brief
     */
    Result<sqlite3_stmt*> prepare(const std::string& sql);

    /**
     * @brief
     */
    VoidResult execute(sqlite3_stmt* stmt);

    VoidResult verify_tables();

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    sqlite3* _db = nullptr;
    std::string _db_path;
    unsigned char* _key = nullptr;
    size_t _key_num_byte = 0;

    std::string _read_scheme_script(const std::string& sql_script_path) const;
    void _init(const std::string_view path, const std::string_view encryption_key);

    VoidResult _new_db();

    void _setup_drp();
    bool _handle_open() const;
    bool _handle_close() const;
    bool _handle_exec() const;
    bool _handle_add_scheme() const;
    bool _handle_new_db() const;
};

#endif  // SQL_H