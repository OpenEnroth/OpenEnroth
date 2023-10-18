#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/System.h"

static const char8_t *u8prefix = u8"\u0444\u0430\u0439\u043B"; // "File" in Russian.
static const char16_t *u16prefix = u"\u0444\u0430\u0439\u043B";

#ifdef _WINDOWS
static_assert(sizeof(char16_t) == sizeof(wchar_t));
#endif

UNIT_TEST(System, fopen) {
    winUseUtf8Crt();

    std::u8string u8path = std::u8string(u8prefix) + u8"_fopen";
    std::string path = reinterpret_cast<const char *>(u8path.c_str());

    const char *data = "data";
    const size_t dataSize = 4;

    FILE *f1 = fopen(path.c_str(), "w");
    EXPECT_NE(f1, nullptr);

    size_t written = fwrite(data, dataSize, 1, f1);
    EXPECT_EQ(written, 1);

    int status = fclose(f1);
    EXPECT_EQ(status, 0);

#ifdef _WINDOWS
    std::u16string u16path = std::u16string(u16prefix) + u"_fopen";
    std::wstring wpath = reinterpret_cast<const wchar_t *>(u16path.c_str());

    FILE *f2 = _wfopen(wpath.c_str(), L"r");
    EXPECT_NE(f2, nullptr);

    char buffer[10] = {};
    size_t read = fread(buffer, dataSize, 1, f2);
    EXPECT_EQ(read, 1);
    EXPECT_EQ(std::string_view(buffer), std::string_view(data));

    int status2 = fclose(f2);
    EXPECT_EQ(status2, 0);
#endif

    // Using UTF-8 api directly here.
    EXPECT_TRUE(std::filesystem::exists(u8path));
    EXPECT_TRUE(std::filesystem::remove(u8path));
    EXPECT_FALSE(std::filesystem::exists(u8path));
}

UNIT_TEST(System, filesystem_exists_remove) {
    winUseUtf8Crt();

    std::u8string u8path = std::u8string(u8prefix) + u8"_exists";
    std::string path = reinterpret_cast<const char *>(u8path.c_str());

    FileOutputStream s(path);
    s.write("something");
    s.close();

    // Using char * api here, expecting it to be handled as UTF-8.
    EXPECT_TRUE(std::filesystem::exists(path));
    EXPECT_TRUE(std::filesystem::remove(path));
    EXPECT_FALSE(std::filesystem::exists(path));
}

UNIT_TEST(System, filesystem_rename) {
    winUseUtf8Crt();

    std::u8string u8path = std::u8string(u8prefix) + u8"_rename";
    std::string path = reinterpret_cast<const char *>(u8path.c_str());
    std::string path2 = path + "2";

    FileOutputStream s(path);
    s.write("something_else");
    s.close();

    // Using char * api here, expecting it to be handled as UTF-8.
    EXPECT_TRUE(std::filesystem::exists(path));
    std::filesystem::rename(path, path2);
    EXPECT_FALSE(std::filesystem::exists(path));
    EXPECT_TRUE(std::filesystem::exists(path2));
    EXPECT_TRUE(std::filesystem::remove(path2));
    EXPECT_FALSE(std::filesystem::exists(path2));
}

UNIT_TEST(System, fstreams) {
    winUseUtf8Crt();

    std::u8string u8path = std::u8string(u8prefix) + u8"_fstreams";
    std::string path = reinterpret_cast<const char *>(u8path.c_str());

    const char *data = "data";
    size_t dataSize = 4;

    std::ofstream f1;
    f1.open(path);
    f1.write(data, dataSize);
    f1.close();

    std::ifstream f2;
    f2.open(path);
    char buffer[10] = {};
    f2.read(buffer, dataSize);
    f2.close();

    EXPECT_EQ(std::string_view(buffer), std::string_view(data));

    // Using UTF-8 api directly here.
    EXPECT_TRUE(std::filesystem::exists(u8path));
    EXPECT_TRUE(std::filesystem::remove(u8path));
    EXPECT_FALSE(std::filesystem::exists(u8path));
}

#ifdef _WINDOWS
UNIT_TEST(System, getenv) {
    winUseUtf8Crt();

    const char *name = "_SOME_VAR_12345";
    const char *value = reinterpret_cast<const char *>(u8prefix);

    const wchar_t *wname = L"_SOME_VAR_12345";
    const wchar_t *wvalue = reinterpret_cast<const wchar_t *>(u16prefix);

    errno_t status = _wputenv_s(wname, wvalue);
    EXPECT_EQ(status, 0);

    std::string result = u8getenv(name);
    EXPECT_EQ(std::string_view(result), std::string_view(value));
}
#endif
