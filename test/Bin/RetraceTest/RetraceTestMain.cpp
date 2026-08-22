#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Application/Startup/GameStarter.h"

#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/String/Format.h"
#include "Utility/System/Os.h"
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

        std::vector<NativePath> traceNames;
        for (const DirectoryEntry &entry : os::ls(opts.testPath))
            if (entry.name.ends_with(".json"))
                traceNames.push_back(NativePath::fromWtf8(entry.name));
        std::ranges::sort(traceNames);

        for (const NativePath &traceName : traceNames)
            testing::RegisterTest("Retrace", traceName.withExtension("").toWtf8().c_str(),
                                  nullptr, nullptr, __FILE__, __LINE__,
                                  [tracePath = opts.testPath / traceName] {
                                      return new RetraceTest(tracePath);
                                  });

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
