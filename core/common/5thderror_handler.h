#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <optional>
#include <variant>

#include "5thdlogger.h"


#define PANIC(msg) throw std::runtime_error(msg)

/**
 * @brief Global error code for 5thd application may be splitted by context in future
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
    MANAGE_BUFF_FULL,
    MANAGE_BUFF_OVERFLOW,
    MANAGE_BUFF_MONKEY,
    MANAGE_BUFF_NULL_ON_RELEASE,
    MANAGE_BUFF_SLOT_NOT_IN_RANGE,
    MONKEY,
    TOTAL
};

/**
 * @brief Error severity for disaster recover stuff
 */
enum class Severity { LOW, MEDIUM, HIGH, CRITICAL };

/**
 * @brief Error object holds the error information
 */
class Error {
public:
    Error(ErrorCode code, std::string message, Severity severity = Severity::HIGH)
        : code_(code), message_(std::move(message)), severity_(severity) {}
    /**
     * @brief method
     * @return enum ErrorCode the code
     */
    ErrorCode code() const { return code_; }
    /**
     * @brief method
     * @return std::string Error message
     */
    const std::string& message() const { return message_; }
    /**
     * @brief method
     * @return enum Severity the code
     */
    Severity severity() const { return severity_; }

private:
    ErrorCode code_;
    std::string message_;
    Severity severity_;
};

/**
 * @brief Automatic checking if error exist
 */
template<typename T>
class Result {
public:
    Result(T value) : value_(std::move(value)) {}
    Result(Error error) : error_(std::move(error)) {}
    bool is_ok() const { return !error_.has_value(); }
    bool is_err() const { return error_.has_value(); }
    const T& value() const {
        if (error_) throw std::runtime_error("Result contains an error");
        return *value_;
    }
    const Error& error() const {
        if (!error_) throw std::runtime_error("Result does not contain an error");
        return *error_;
    }
private:
    std::optional<T> value_;
    std::optional<Error> error_;
};

template<typename T>
Result<T> Ok(T value) {
    return Result<T>(std::move(value));
};

template <typename T>
Result<T> Err(ErrorCode code, const std::string& message, Severity severity = Severity::MEDIUM) {
    return Result<T>(Error(code, message, severity));
};

class VoidResult {
public:
    VoidResult() : state_(std::monostate{}) {}
    VoidResult(Error error) : state_(std::move(error)) {}

    bool is_ok() const { return std::holds_alternative<std::monostate>(state_); }
    bool is_err() const { return std::holds_alternative<Error>(state_); }

    void value() const {
        if (is_err()) throw std::runtime_error("Result contains an error");
    }

    const Error& error() const {
        if (is_ok()) throw std::runtime_error("Result does not contain an error");
        return std::get<Error>(state_);
    }

private:
    std::variant<std::monostate, Error> state_;
};

inline VoidResult Ok() {
    return VoidResult();
};

inline VoidResult Err(ErrorCode code, const std::string& message, Severity severity = Severity::MEDIUM) {
    return VoidResult(Error(code, message, severity));
};

/**
 * @brief Class will hold DRP callbacks for specific error
 * @note in case of fail will throw std::runtime_error();
 */
class DisasterRecoveryPlan {
public:
    /**
     * @brief c++ equivalent of c typedef for callback bool(recovery_action*)()
     */
    using RecoveryAction = std::function<bool()>;
    /**
     * @brief Register the callback on error
     */
    void register_recovery_action(ErrorCode code, RecoveryAction action) {
        recovery_actions_[code] = std::move(action);
    }
    /**
     * @brief Actual recovery call
     * @return bool
     */
    bool execute_recovery(const Error& error) {
        auto it = recovery_actions_.find(error.code());
        if (it != recovery_actions_.end()) {
            return it->second();
        }
        return false;
    }

private:
    std::unordered_map<ErrorCode, RecoveryAction> recovery_actions_;
};

/**
 * @brief Error handler class handles automaticaly
 */
class ErrorHandler {
public:
    /**
     * @brief c++ equivalent of c typedef for callback void(const Error*)()
     */
    using ErrorCallback = std::function<void(const Error&)>;

    ErrorHandler(DisasterRecoveryPlan& drp) : drp_(drp) {}

    /**
     * @brief Register a callback on specific erro
     * @param ErrorCode the error code
     * @paragraph ErrorCallback the function you wanna call on it
     */
    void register_callback(ErrorCode code, ErrorCallback callback) { callbacks_[code] = std::move(callback); }

    /**
     * @brief Handles error
     * @note Will call DRP action if registered on particular error
     * @note Also can be added independet callback on particular error
     * @return bool in case of DRP fail will throw runtime_error
     */
    bool handle_error(const Error& error) {
        auto it = callbacks_.find(error.code());
        if (it != callbacks_.end()) {
            it->second(error);
        }

        bool recovered = false;
        if (error.severity() >= Severity::HIGH) {
            recovered = drp_.execute_recovery(error);
            if (!recovered && error.severity() == Severity::CRITICAL) {
                throw std::runtime_error("Critical error: " + error.message());
            }
        }

        log_error(error);
        return recovered;
    }

private:
    std::unordered_map<ErrorCode, ErrorCallback> callbacks_;
    DisasterRecoveryPlan& drp_;

    void log_error(const Error& error) {
        ERROR("[ Severity: {} Error code: {} Error message: {} ]", static_cast<int>(error.severity()),
              static_cast<int>(error.code()), error.message());
    }
};

#endif  // ERROR_HANDLER_H