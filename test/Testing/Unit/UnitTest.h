#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Testing/Extensions/ThrowingAssertions.h"
#include "Testing/Extensions/ExpectExtensions.h"
#include "Testing/Extensions/ScopedTestFile.h"
#include "Testing/Extensions/ScopedTestFileSlot.h"

#define UNIT_TEST(SuiteName, TestName) \
    TEST(SuiteName, TestName)

#define UNIT_TEST_FIXTURE(ClassName, TestName) \
    TEST_F(ClassName, TestName)
