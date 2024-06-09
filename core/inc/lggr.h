#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

class Log {
public:
    static void init();
    static std::shared_ptr<spdlog::logger> get_logger();

private:
    static std::shared_ptr<spdlog::logger> _logger;
};

#define DEBUG(...) Log::get_logger()->debug(__VA_ARGS__)
#define ERROR(...) Log::get_logger()->error(__VA_ARGS__)

#endif  // LOGGER_MANAGER_H