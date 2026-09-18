#pragma once

#include "Utility/System/NativePath.h"

struct IntegrationTestOptions {
    NativePath binaryPath;
    bool helpPrinted = false;
    bool listRequested = false;

    static IntegrationTestOptions parse(int argc, char **argv);
};
