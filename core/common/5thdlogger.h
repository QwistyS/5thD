#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

#include "spdlog/spdlog.h"

class Log {
public:
    static void init();
    static std::shared_ptr<spdlog::logger> get_logger();

private:
    static std::shared_ptr<spdlog::logger> _logger;
};

// Define logging macros
#ifndef NDEBUG
    // Debug build
    #define DEBUG(...) Log::get_logger()->debug(__VA_ARGS__)
#else
    // Release build
    #define DEBUG(...) (void)0
#endif

#define ERROR(...) Log::get_logger()->error(__VA_ARGS__)
#define WARN(...) Log::get_logger()->warn(__VA_ARGS__)

#endif  // LOGGER_MANAGER_H
