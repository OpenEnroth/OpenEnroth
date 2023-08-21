#include <gtest/gtest.h>

#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/UnicodeCrt.h"

GTEST_API_ int main(int argc, char **argv) {
    StackTraceOnCrash st;
    UnicodeCrt _(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
