#include <string>

void SetDataPath(const std::string &data_path);

std::string MakeDataPath(std::initializer_list<std::string_view> paths);
template<typename ... Ts>
std::string MakeDataPath(Ts&&... paths) {
    static_assert(((std::is_same<typename std::decay<Ts>::type, std::string>::value ||
        std::is_same<typename std::decay<Ts>::type, const char*>::value) || ...),
        "T must be a basic string");
    return MakeDataPath({ paths... });
}

std::string MakeTempPath(const char *file_rel_path);

