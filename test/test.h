#include <gtest/gtest.h>

#ifdef TESTGROUP
#define GTEST(TestBase, TestName) TEST(TESTGROUP, TestBase##_##TestName)
#else
#define GTEST TEST
#endif
