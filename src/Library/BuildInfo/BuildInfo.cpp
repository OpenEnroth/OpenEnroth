#include "BuildInfo.h"

#include "BuildInfo.gen.h"

std::string_view gitRevision() {
    return MM_GIT_REVISION;
}

std::string_view buildTime() {
    return MM_BUILD_TIME;
}
