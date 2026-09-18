#include "IntegrationTest.h"

#include <cassert>
#include <chrono>
#include <string>
#include <vector>

#include "Utility/String/Format.h"

static NativePath globalBinaryPath;

static NativePath testFolderPath() {
    const testing::TestInfo *info = testing::UnitTest::GetInstance()->current_test_info();
    return NativePath::fromWtf8(fmt::format("{}.{}", info->test_suite_name(), info->name())).absolute();
}

IntegrationTest::IntegrationTest() : _userPath(testFolderPath()), _userFolder(_userPath) {}

void IntegrationTest::init(const NativePath &binaryPath) {
    assert(globalBinaryPath.isEmpty() && !binaryPath.isEmpty());
    globalBinaryPath = binaryPath;
}

ProcessResult IntegrationTest::runOpenEnroth(const std::vector<std::string> &args) const {
    return runProcess(globalBinaryPath, args, std::chrono::minutes(2)); // Only a hung game gets anywhere near this.
}
