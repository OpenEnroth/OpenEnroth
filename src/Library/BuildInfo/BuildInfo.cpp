#include "BuildInfo.h"

#ifndef MM_GIT_REVISION
#include "Library/BuildInfo/BuildInfoStamp.h" // Bazel stamps via a generated header, cmake passes the macros on the command line.
#endif

std::string_view gitRevision() {
    return MM_GIT_REVISION;
}

std::string_view buildTime() {
    return MM_BUILD_TIME;
}

std::string_view buildVersion() {
    return PROJECT_VERSION;
}
