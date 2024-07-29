#include <cstdlib>
#include <memory>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Library/Environment/Interface/Environment.h"

static const char8_t *u8prefix = u8"\u0444\u0430\u0439\u043B"; // "File" in Russian.
static const char16_t *u16prefix = u"\u0444\u0430\u0439\u043B";

#ifdef _WINDOWS
static_assert(sizeof(char16_t) == sizeof(wchar_t));
#endif

UNIT_TEST(Environment, getenv_empty) {
    std::unique_ptr<Environment> environment = Environment::createStandardEnvironment();

    std::string result = environment->getenv("_ABCDEFG_123456_"); // Getting a non-existent var should work.
    EXPECT_TRUE(result.empty());
}

UNIT_TEST(Environment, getenv) {
    std::unique_ptr<Environment> environment = Environment::createStandardEnvironment();

    const char *name = "_SOME_VAR_12345";
    const char *value = reinterpret_cast<const char *>(u8prefix);

    const wchar_t *wname = L"_SOME_VAR_12345";
    const wchar_t *wvalue = reinterpret_cast<const wchar_t *>(u16prefix);

#ifdef _WINDOWS
    errno_t status = _wputenv_s(wname, wvalue);
    EXPECT_EQ(status, 0);
#else
    int status = setenv(name, value, 1);
    EXPECT_EQ(status, 0);
#endif

    std::string ourResult = environment->getenv(name);
    EXPECT_EQ(std::string_view(ourResult), std::string_view(value));

    const char *stdResultPtr = std::getenv(name);
    std::string stdResult = stdResultPtr ? stdResultPtr : "";
    EXPECT_NE(stdResultPtr, nullptr);

#ifdef _WINDOWS
    EXPECT_EQ(stdResult, "????"); // Unfortunately, this is how it works. We just pin it in place with a test.
#else
    EXPECT_EQ(stdResult, value);
#endif
}
