#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "Application/Startup/GameStarter.h"

#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"

#include "RetraceTest.h"
#include "RetraceTestOptions.h"

int platformMain(int argc, char **argv) {
    try {
        StackTraceOnCrash st;
        UnicodeCrt _(argc, argv);
        RetraceTestOptions opts = RetraceTestOptions::parse(argc, argv);
        if (opts.helpPrinted)
            return 1;

        std::vector<std::string> tracePaths;
        for (const auto &entry : std::filesystem::directory_iterator(opts.testPath))
            if (entry.path().extension() == ".json")
                tracePaths.push_back(entry.path().string());
        std::ranges::sort(tracePaths);

        for (const std::string &tracePath : tracePaths)
            testing::RegisterTest("Retrace", std::filesystem::path(tracePath).stem().string().c_str(),
                                  nullptr, nullptr, __FILE__, __LINE__,
                                  [tracePath] { return new RetraceTest(tracePath); });

        testing::InitGoogleTest(&argc, argv);
        if (opts.listRequested)
            return RUN_ALL_TESTS();

        GameStarter starter(opts);
        int exitCode = 0;
        starter.runInstrumented([&] (EngineController *game) {
            RetraceTest::init(game, starter.application());
            exitCode = RUN_ALL_TESTS();
        });
        return exitCode;
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
