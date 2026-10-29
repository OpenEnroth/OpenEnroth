#include "IntegrationTest.h"

#include <cassert>
#include <chrono>
#include <string>
#include <vector>

#include "Utility/String/Format.h"
#include "Utility/System/Fs.h"

static Path globalBinaryPath;

static Path testFolderPath() {
    const testing::TestInfo *info = testing::UnitTest::GetInstance()->current_test_info();
    return fs::absolute(Path(fmt::format("{}.{}", info->test_suite_name(), info->name())));
}

IntegrationTest::IntegrationTest() : _userPath(testFolderPath()), _userFolder(_userPath) {}

void IntegrationTest::init(const Path &binaryPath) {
    assert(globalBinaryPath.isEmpty() && !binaryPath.isEmpty());
    globalBinaryPath = binaryPath;
}

ProcessResult IntegrationTest::runOpenEnroth(const std::vector<std::string> &args) const {
    ProcessResult result = runProcess(globalBinaryPath, args, std::chrono::minutes(2)); // Only a hung game gets anywhere near this.
    ASSERT_FALSE(result.timedOut) << result.stdErr;
    return result;
}
