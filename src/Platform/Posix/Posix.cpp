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
#include "Platform/Sdl/SdlPlatform.h"

//////////////////// There is no Windows Registry ////////////////////

std::unique_ptr<Platform> Platform::CreateStandardPlatform(Log *log) {
    return std::make_unique<SdlPlatform>(log);
}
