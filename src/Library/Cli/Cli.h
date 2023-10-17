#pragma once

#include <utility>
#include <string>

#include <CLI/CLI.hpp>

/**
 * A thin wrapper around `CLI:App` that simplifies the code for our use cases.
 */
class CliApp : public CLI::App {
    using base_type = CLI::App;
 public:
    CliApp() = default;

    template<class SubcommandEnum>
    CliApp *add_subcommand(std::string name, std::string description, SubcommandEnum &target, SubcommandEnum targetValue) {
        auto callback = [target = &target, targetValue] {
            *target = targetValue;
        };
        return static_cast<CliApp *>(base_type::add_subcommand(std::move(name), std::move(description))->callback(std::move(callback)));
    }

    void parse(int argc, const char *const *argv, bool &helpPrinted);
};
