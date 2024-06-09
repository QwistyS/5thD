#include "lggr.h"

std::shared_ptr<spdlog::logger> Log::_logger;

void Log::init() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks{console_sink};

    _logger = std::make_shared<spdlog::logger>("multi_sink", begin(sinks), end(sinks));
    _logger->set_level(spdlog::level::debug);
    _logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    spdlog::register_logger(_logger);
    spdlog::set_default_logger(_logger);
}

std::shared_ptr<spdlog::logger> Log::get_logger() {
    return _logger;
}