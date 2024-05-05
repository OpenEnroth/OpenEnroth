#include "LoggerBindings.h"

#include "Library/Logger/Logger.h"

sol::table LoggerBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "info", [](std::string_view message) { logger->info("{}", message); },
        "trace", [](std::string_view message) { logger->trace("{}", message); },
        "debug", [](std::string_view message) { logger->debug("{}", message); },
        "warning", [](std::string_view message) { logger->warning("{}", message); },
        "error", [](std::string_view message) { logger->error("{}", message); },
        "critical", [](std::string_view message) { logger->critical("{}", message); }
    );
}
