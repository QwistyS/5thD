#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <cstdarg>
#include <cstdio>
#include <vector>

// Route QWISTYS_ERROR_MSG through 5thD's spdlog-based logger.
// Must be defined before qwistys_error_handler.h is included.
#include "5thdlogger.h"

inline void qwistys_error_msg_printf(const char* msg, ...) {
    if (msg == nullptr) {
        return;
    }

    va_list args;
    va_start(args, msg);

    va_list args_copy;
    va_copy(args_copy, args);
    const int size = std::vsnprintf(nullptr, 0, msg, args_copy);
    va_end(args_copy);

    if (size < 0) {
        va_end(args);
        std::fprintf(stderr, "[ERROR] %s\n", msg);
        return;
    }

    std::vector<char> buffer(static_cast<size_t>(size) + 1U);
    std::vsnprintf(buffer.data(), buffer.size(), msg, args);
    va_end(args);

    if (auto logger = Log::get_logger()) {
        logger->error("{}", buffer.data());
        return;
    }

    std::fprintf(stderr, "[ERROR] %s\n", buffer.data());
}

#define QWISTYS_ERROR_MSG(...) qwistys_error_msg_printf(__VA_ARGS__)

#define PANIC(msg) throw std::runtime_error(msg)

/**
 * @brief 5thD-specific error codes.
 */
enum class ErrorCode {
    OK = 0,
    SOCKET_INIT_FAIL,
    SOCKET_CLOSE_FAIL,
    SOCKET_SEND_FAIL,
    SOCKET_TIMEOUT,
    SOCKET_CONNECT_FAIL,
    FAIL_POLL_SOCKET,
    SEND_HEARTBEAT_FAIL,
    FAIL_CLOSE_ZQM_CTX,
    FAIL_CLOSE_ZQM_SOCKET,
    FAIL_OPEN_ZMQ_CTX,
    FAIL_OPEN_SOCKET,
    FAIL_TO_BIND,
    FAIL_BIND_SOCKET,
    FAIL_INIT_LISTENER,
    FAIL_INIT_KEYS,
    FAIL_SET_SOCKOPT,
    FAIL_SETSOCKOPT_ID,
    FAIL_UPNP_FORWARD,
    FIAIL_RECV_MSG,
    FAIL_SEND_FRAME,
    NO_OBJECT,
    OBJ_RECEIVER_INIT_FAIL,
    FAIT_INIT_DB,
    FAIL_CREATE_DB_SCHEME,
    FAIL_OPEN_DB_FILE,
    FAIL_DECRYPT_DB,
    DB_PREPARE_STATEMENT_FAILED,
    DB_STATEMENT_EXECUTION_FAILED,
    DB_BIND_FAILED,
    KEY_NOT_FOUND,
    FAIL_ADD_KEY,
    FAIL_UPDATE_KEY,
    FAIL_GET_KEY,
    FAIL_REMOVE_KEY,
    KEY_TYPE_NOT_FOUND,
    NO_VALID_DB,
    DB_MISSING_TABLES,
    FAIL_DB_EXEC,
    DB_ERROR,
    FAIL_CLOSE_DB,
    INVALID_IDENTITY,
    MANAGE_BUFF_FULL,
    MANAGE_BUFF_OVERFLOW,
    MANAGE_BUFF_MONKEY,
    MANAGE_BUFF_NULL_ON_RELEASE,
    MANAGE_BUFF_SLOT_NOT_IN_RANGE,
    MONKEY,
    TOTAL
};

#include "qwistys_error_handler.h"

#endif  // ERROR_HANDLER_H
