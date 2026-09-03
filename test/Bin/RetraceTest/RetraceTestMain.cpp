#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Application/Startup/GameStarter.h"

#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Exception.h"
#include "Utility/String/Format.h"
#include "Utility/System/Fs.h"
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

        if (fs::stat(opts.testPath).type != FILE_DIRECTORY)
            throw Exception("Test path '{}' is not a directory", opts.testPath);

        std::vector<Path> traceNames;
        for (const DirectoryEntry &entry : fs::ls(opts.testPath))
            if (entry.name.ends_with(".json"))
                traceNames.push_back(Path(entry.name));
        std::ranges::sort(traceNames);

        if (traceNames.empty())
            throw Exception("No traces found in '{}'", opts.testPath); // Or we'd exit green having run nothing.

        for (const Path &traceName : traceNames)
            testing::RegisterTest("Retrace", traceName.withExtension("").string().c_str(),
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
