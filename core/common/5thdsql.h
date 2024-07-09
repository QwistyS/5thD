#ifndef SQL_H
#define SQL_H

#define SQLITE_HAS_CODEC 1

#include <sodium.h>
#include <sqlcipher/sqlite3.h>
#include <memory>
#include <string>
#include <vector>

#include "5thderror_handler.h"

// in future will move to env variable

#ifndef DB_PATH
#define DB_PATH "/home/qwistys/src/5thD/bin/fithd.db"
#endif

inline constexpr char MEANWHILE_DB_KEY[] = "It was meant to be but not meant to last";

typedef struct SecureQueryResult {
    struct Row {
        std::vector<std::pair<std::string, std::vector<unsigned char>>> columns;
    };
    std::vector<Row> rows;
    size_t totalSize;

    SecureQueryResult() : totalSize(0) {}

    ~SecureQueryResult() { flush(); }

    void flush() {
        for (auto& row : rows) {
            for (auto& col : row.columns) {
                sodium_memzero(col.second.data(), col.second.size());
            }
        }
        rows.clear();
        totalSize = 0;
    }

} sdbret_t;

class DatabaseAccess {
public:
    DatabaseAccess(const std::string& path = DB_PATH, const std::string& encryption_key = MEANWHILE_DB_KEY)
        : _db_path(path), _key(nullptr), _key_num_byte(0), _error(_drp), _db(nullptr) {
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

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    sqlite3* _db;
    std::string _db_path;
    unsigned char* _key;
    size_t _key_num_byte;

    std::string _read_scheme_script(const std::string& sql_script_path);
    void _init(const std::string& path, const std::string encryption_key);

    VoidResult _new_db();

    void _setup_drp();
    bool _handle_open();
    bool _handle_close();
    bool _handle_exec();
    bool _handle_add_scheme();
    bool _handle_new_db();
};

#endif  // SQL_H