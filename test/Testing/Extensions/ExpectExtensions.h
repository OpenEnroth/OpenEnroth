#pragma once

#include <exception>
#include <string_view>

#include <gtest/gtest.h> // NOLINT: not a C system header.

#define EXPECT_THROW_MESSAGE(statement, message)                                                                        \
    try {                                                                                                               \
        statement;                                                                                                      \
        EXPECT_TRUE(false) << #statement << " didn't throw.";                                                           \
    } catch (const std::exception &e) {                                                                                 \
        EXPECT_TRUE(std::string_view(e.what()).contains(message))                                                       \
            << "exception message '" << e.what() << "' doesn't have '" << message << "' as a substring.";               \
    } catch (...) {                                                                                                     \
        EXPECT_TRUE(false) << #statement << " has thrown an unknown exception.";                                        \
    }

// TODO(captainurist) : EXPECT_CONTAINS
