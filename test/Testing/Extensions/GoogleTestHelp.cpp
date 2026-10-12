#include "GoogleTestHelp.h"

#include <gtest/gtest.h>

#include "Utility/String/Format.h"

void printGoogleTestHelp(char *app) {
    fmt::print(stdout, "\n");

    int argc = 2;
    char help[] = "--help";
    char *argv[] = { app, help, nullptr };
    testing::InitGoogleTest(&argc, argv);
}
