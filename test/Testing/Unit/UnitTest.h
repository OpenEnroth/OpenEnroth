#include <gtest/gtest.h>

#define UNIT_TEST(SuiteName, TestName) \
    TEST(SuiteName, TestName)

#define UNIT_TEST_FIXTURE(ClassName, TestName) \
    TEST_F(ClassName, TestName)
