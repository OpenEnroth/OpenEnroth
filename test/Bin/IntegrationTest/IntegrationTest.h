#pragma once

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Testing/Extensions/ScopedTestFolder.h"

#include "Utility/System/NativePath.h"
#include "Utility/System/Process.h"

/**
 * Base for tests that run the OpenEnroth binary as a separate process. Every test gets an empty folder of its own
 * to use as the user path.
 */
class IntegrationTest : public testing::Test {
 public:
    IntegrationTest();

    static void init(const NativePath &binaryPath);

 protected:
    [[nodiscard]] ProcessResult runOpenEnroth(const std::vector<std::string> &args) const;

    [[nodiscard]] const NativePath &userPath() const {
        return _userPath;
    }

 private:
    NativePath _userPath;
    ScopedTestFolder _userFolder;
};
