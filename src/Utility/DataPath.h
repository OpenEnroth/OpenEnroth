#include <string>
#include <type_traits>

void SetDataPath(const std::string &data_path);

std::string MakeDataPath(std::initializer_list<std::string_view> paths);

template<typename... Ts>
std::string MakeDataPath(Ts&&... paths) {
    static_assert(((std::is_same_v<std::remove_cvref_t<Ts>, std::string> ||
        std::is_same_v<std::decay_t<Ts>, const char *>) && ...),
        "T must be a basic string");
    return MakeDataPath({ paths... });
}

std::string MakeTempPath(const char *file_rel_path);
bool ValidataDataPath(const std::string &data_path);
