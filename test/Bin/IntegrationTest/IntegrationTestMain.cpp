#include <gtest/gtest.h>

#include <exception>

#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"

#include "IntegrationTest.h"
#include "IntegrationTestOptions.h"

int main(int argc, char **argv) {
    try {
        StackTraceOnCrash st;
        UnicodeCrt _(argc, argv);
        IntegrationTestOptions opts = IntegrationTestOptions::parse(argc, argv);
        if (opts.helpPrinted)
            return 1;

        testing::InitGoogleTest(&argc, argv);
        if (!opts.listRequested)
            IntegrationTest::init(opts.binaryPath);
        return RUN_ALL_TESTS();
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
