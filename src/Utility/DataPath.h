#pragma once

#include <string>
#include <type_traits>
#include <filesystem>

// TODO(captainurist): this doesn't belong in Utility

void setDataPath(const std::filesystem::path &dataPath);

std::string makeDataPath(std::initializer_list<std::string_view> paths);

template<typename... Ts>
std::string makeDataPath(Ts&&... paths) {
    static_assert(((std::is_same_v<std::remove_cvref_t<Ts>, std::string> || std::is_same_v<std::decay_t<Ts>, const char *> || std::is_same_v<std::decay_t<Ts>, std::string_view>) && ...),
                  "T must be a basic string");
    return makeDataPath({paths...});
}

bool validateDataPath(std::string_view data_path, std::string *missing_file);
