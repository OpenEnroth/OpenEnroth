#pragma once

#include <exception>
#include <string_view>

#include <gtest/gtest.h> // NOLINT: not a C system header.

#define EXPECT_THROW_MESSAGE(STATEMENT, MESSAGE)                                                                        \
    do {                                                                                                                \
        try {                                                                                                           \
            STATEMENT;                                                                                                  \
            EXPECT_TRUE(false) << #STATEMENT << " didn't throw.";                                                       \
        } catch (const std::exception &e) {                                                                             \
            EXPECT_CONTAINS(std::string_view(e.what()), (MESSAGE));                                                     \
        } catch (...) {                                                                                                 \
            EXPECT_TRUE(false) << #STATEMENT << " has thrown an unknown exception.";                                    \
        }                                                                                                               \
    } while (0)

#define EXPECT_CONTAINS(HAYSTACK, NEEDLE)                                                                               \
    do {                                                                                                                \
        const auto &__haystack = (HAYSTACK);                                                                            \
        const auto &__needle = (NEEDLE);                                                                                \
        EXPECT_TRUE(__haystack.contains(__needle))                                                                      \
            << #HAYSTACK << " (" << testing::PrintToString(__haystack) << ") doesn't contain "                          \
            << #NEEDLE << " (" << testing::PrintToString(__needle) << ")";                                              \
    } while (0)

#define EXPECT_MISSES(HAYSTACK, NEEDLE)                                                                                 \
    do {                                                                                                                \
        const auto &__haystack = (HAYSTACK);                                                                            \
        const auto &__needle = (NEEDLE);                                                                                \
        EXPECT_FALSE(__haystack.contains(__needle))                                                                     \
            << #HAYSTACK << " (" << testing::PrintToString(__haystack) << ") contains "                                 \
            << #NEEDLE << " (" << testing::PrintToString(__needle) << ")";                                              \
    } while (0)
