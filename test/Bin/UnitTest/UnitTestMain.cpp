#include <gtest/gtest.h>

#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/UnicodeCrt.h"

GTEST_API_ int main(int argc, char **argv) {
    StackTraceOnCrash st(nullptr, STACK_TRACE_LOAD_SYMBOLS_ON_CRASH);
    UnicodeCrt _(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
