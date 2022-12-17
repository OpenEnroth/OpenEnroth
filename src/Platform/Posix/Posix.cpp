#include "Platform/Posix/Posix.h"

#include <SDL.h>
#include <dirent.h>
#include <glob.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>

#include "Utility/String.h"

bool OS_OpenConsole() {
    return true;
}

//////////////////// There is no Windows Registry ////////////////////

bool OS_GetAppString(const char* path, char* out_string, int out_string_size) {
    return false;
}
