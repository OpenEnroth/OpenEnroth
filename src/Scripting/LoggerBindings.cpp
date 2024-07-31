#include "LoggerBindings.h"

#include "Library/Logger/Logger.h"

sol::table LoggerBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "info", sol::as_function([](std::string_view message) { logger->info("{}", message); }),
        "trace", sol::as_function([](std::string_view message) { logger->trace("{}", message); }),
        "debug", sol::as_function([](std::string_view message) { logger->debug("{}", message); }),
        "warning", sol::as_function([](std::string_view message) { logger->warning("{}", message); }),
        "error", sol::as_function([](std::string_view message) { logger->error("{}", message); }),
        "critical", sol::as_function([](std::string_view message) { logger->critical("{}", message); })
    );
}
