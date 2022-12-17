#pragma once

#include <gtest/gtest.h>

#ifndef GTEST_FATAL_FAILURE_
#   error "Something changed in Google Test implementation, this hack won't work."
#endif

/*
 * Include this header and all your gtest `ASSERT_*` macros will instantly become better:
 * - Can be used inside any function, not just the ones returning void.
 * - Always stop the current test by throwing an exception.
 *
 * Note that it won't affect ASSERT_* macros in translation units that don't include this header.
 */

namespace detail {
struct ScopedEnableThrowingAssertions {
    ScopedEnableThrowingAssertions();
    ~ScopedEnableThrowingAssertions();
};
}

#undef GTEST_FATAL_FAILURE_
#define GTEST_FATAL_FAILURE_(message) \
    detail::ScopedEnableThrowingAssertions(), GTEST_MESSAGE_(message, ::testing::TestPartResult::kFatalFailure)
