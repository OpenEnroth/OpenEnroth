#include <gtest/gtest.h>

#define GTEST(TestBase, TestName) TEST(TESTGROUP, TestBase##_##TestName)
