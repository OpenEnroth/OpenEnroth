#pragma once

#include "Utility/System/Path.h"

struct IntegrationTestOptions {
    Path binaryPath;
    bool helpPrinted = false;
    bool listRequested = false;

    static IntegrationTestOptions parse(int argc, char **argv);
};
