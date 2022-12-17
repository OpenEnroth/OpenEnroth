#include "DataPath.h"

#include "FileSystem.h"

static std::filesystem::path s_data_path;

void SetDataPath(const std::string &data_path) {
    s_data_path = ExpandUserPath(data_path);
}

std::string MakeDataPath(std::initializer_list<std::string_view> paths) {
    std::filesystem::path result = s_data_path;

    for (auto p : paths)
        if (!p.empty())
            result /= p;

    return MakeCaseInsensitivePath(result).string();
}

std::string MakeTempPath(const char *file_rel_path) {
    return (std::filesystem::temp_directory_path() / file_rel_path).string();
}
