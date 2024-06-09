#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace logger5thd {
    inline void init() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("application.log", true);
        std::vector<spdlog::sink_ptr> sinks{console_sink};

        auto logger = std::make_shared<spdlog::logger>("multi_sink", begin(sinks), end(sinks));
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    }

    inline std::shared_ptr<spdlog::logger> get_logger(const std::string& logger_name) {
        return spdlog::get(logger_name);
    };
}  // namespace logger5thd

#endif  // LOGGER_MANAGER_H