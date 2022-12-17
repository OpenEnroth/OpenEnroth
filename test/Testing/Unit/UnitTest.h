#include <gtest/gtest.h>

#ifndef TEST_GROUP
#   error "Please define TEST_GROUP before including this header."
#endif

#define UNIT_TEST(TestBase, TestName) \
    TEST(TEST_GROUP, TestBase##_##TestName)
