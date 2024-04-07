#include "UnicodeCrt.h"

#include <cassert>

#ifdef _WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include <shellapi.h>

#   include <clocale>
#   include <memory>

#   include "Utility/Win/Unicode.h"
#   include "Utility/Exception.h"

struct LocalFreeDeleter {
    void operator()(HLOCAL ptr) {
        LocalFree(ptr);
    }
};
#endif

static bool globalUnicodeCrtInitialized = false;

UnicodeCrt::UnicodeCrt(int &argc, char **&argv) {
    assert(!globalUnicodeCrtInitialized); // Don't create several instances!
    globalUnicodeCrtInitialized = true;

#ifdef _WINDOWS
    // Convert command line first.
    //
    // Note on SDL interop. SDL runs basically the same code before calling into SDL_main, and thus if we get here from
    // platformMain, argc and argv are already UTF8-encoded. However, SDL doesn't add/remove arguments, so it's safe to
    // run the same code again.
    //
    // CommandLineToArgvW can return NULL when out of memory, which should never happen. We don't handle errors here.
    std::unique_ptr<LPWSTR[], LocalFreeDeleter> argvw(CommandLineToArgvW(GetCommandLineW(), &argc)); // NOLINT

    for (int i = 0; i < argc; i++)
        _storage.push_back(win::toUtf8(argvw[i]));
    for (int i = 0; i < argc; i++)
        _argv.push_back(_storage[i].data());
    _argv.push_back(nullptr);
    argv = _argv.data();

    // Switch to UTF8 for CRT functions. Without this, std::filesystem won't be able to process UTF8 paths.
    if (std::setlocale(LC_ALL, ".UTF-8") == nullptr)
        throw Exception("Could not change system locale to UTF-8");

    // Also use UTF8 for console io.
    if (SetConsoleCP(CP_UTF8) == 0)
        throw Exception("Could not set console input codepage to UTF-8");
    if (SetConsoleOutputCP(CP_UTF8) == 0)
        throw Exception("Could not set console output codepage to UTF-8");
#endif
}

bool UnicodeCrt::isInitialized() {
    return globalUnicodeCrtInitialized;
}
