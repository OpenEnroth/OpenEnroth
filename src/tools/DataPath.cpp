#include "src/tools/DataPath.h"

#include <filesystem>

#include "Platform/Api.h"

static std::string s_data_path;

void SetDataPath(const std::string &data_path) { s_data_path = data_path; }

std::string MakeDataPath(std::initializer_list<std::string_view> paths) {
    std::string res = s_data_path;
    std::string sep;

    sep.push_back(OS_GetDirSeparator());

    for (auto p : paths) {
        if (!p.empty()) {
            if (!res.empty())
                res += sep;

            res += p;
        }
    }

    res = OS_casepath(res);

    return res;
}

std::string MakeTempPath(const char *file_rel_path) {
    return std::filesystem::temp_directory_path().string() + OS_GetDirSeparator() + file_rel_path;
}
