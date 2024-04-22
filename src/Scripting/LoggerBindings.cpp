#include "LoggerBindings.h"

#include <Library/Logger/Logger.h>

LoggerBindings::LoggerBindings(const sol::state_view &solState) : _solState(solState) {
}

void LoggerBindings::init() {
    _solState.set_function("initLogger", [this]() {
        sol::table mainTable = _solState.create_table_with(
            "info", [](std::string_view message) { logger->info(message); },
            "trace", [](std::string_view message) { logger->trace(message); },
            "debug", [](std::string_view message) { logger->debug(message); },
            "warning", [](std::string_view message) { logger->warning(message); },
            "error", [](std::string_view message) { logger->error(message); },
            "critical", [](std::string_view message) { logger->critical(message); }
        );
        return mainTable;
    });
}
