#include <gtest/gtest.h>

#include "Application/GameStarter.h"
#include "Application/GameConfig.h"

#include "Engine/Components/Control/EngineControlComponent.h"
#include "Engine/Components/Control/EngineController.h"

#include "Testing/Game/GameTest.h"
#include "Testing/Game/TestController.h"

#include "Library/StackTrace/StackTraceOnCrash.h"
#include "Library/Platform/Application/PlatformApplication.h"

#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"

#include "GameTestOptions.h"

#ifndef __ANDROID__
#   include <cpptrace/cpptrace.hpp>
#endif

void printGoogleTestHelp(char *app) {
    int argc = 2;
    char help[] = "--help";
    char *argv[] = { app, help, nullptr };
    testing::InitGoogleTest(&argc, argv);
}

int platformMain(int argc, char **argv) {
    try {
        StackTraceOnCrash st;
        UnicodeCrt _(argc, argv);
        GameTestOptions opts = GameTestOptions::parse(argc, argv);
        if (opts.helpPrinted) {
            fmt::print(stdout, "\n");
            printGoogleTestHelp(argv[0]);
            return 1;
        }

        testing::InitGoogleTest(&argc, argv);
        if (opts.listRequested)
            return RUN_ALL_TESTS();

        GameStarter starter(opts);

        int exitCode = 0;
        starter.runInstrumented([&] (EngineController *game) {
            TestController test(game, opts.testPath, opts.speed);
            GameTest::init(game, &test);
            exitCode = RUN_ALL_TESTS();
        });
        return exitCode;
    } catch(cpptrace::exception& e) {
        // Prints the exception info and stack trace, conditionally enabling color codes depending on
        // whether stderr is a terminal
        std::cerr << "Error: " << e.message() << '\n';
        e.trace().print(std::cerr, cpptrace::isatty(cpptrace::stderr_fileno));        
    } catch (const std::exception &e) {
        // TODO(captainurist): we need a separate test that testing framework terminates correctly if the engine throws.
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
