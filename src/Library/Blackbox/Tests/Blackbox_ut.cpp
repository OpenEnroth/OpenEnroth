#include <cstdlib>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Blackbox/Blackbox.h"
#include "Library/StackTrace/StackTraceOnCrash.h"

#include "Utility/Attributes.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Split.h"

#ifndef __ANDROID__ // No crash handlers on android, so nothing ever reaches the file.

static const NativePath logPath("blackbox_ut.log"); // Relative, a death test child runs in the same directory.
static const NativePath oldLogPath("blackbox_ut.log.old");

/**
 * Every test here runs the blackbox in a death test child. It registers a process-wide callback and holds its
 * file open for good, so the parent never constructs one - a second instance asserts, and on windows the
 * parent couldn't remove the file afterwards.
 */
class BlackboxTest : public testing::Test {
 protected:
    std::vector<std::string> logLines() {
        std::vector<std::string> result;
        Blob log = Blob::fromFile(logPath);
        for (std::string_view line : split(log.str()).by('\n'))
            if (!line.empty())
                result.emplace_back(line);
        return result;
    }

 private:
    ScopedTestFileSlot _logSlot{logPath};
    ScopedTestFileSlot _oldLogSlot{oldLogPath};
};

MM_NOINLINE int blackboxCrashingFunction() {
    // Faults at a small address rather than at zero, so that the code differs from the null write in the stack
    // trace tests - the windows linker folds identical functions into one, and the trace then names whichever
    // survived.
    int *volatile nowhere = reinterpret_cast<int *>(64); // Volatile, or the store is folded away.
    *nowhere = 1;
    return *nowhere; // The read feeds the return value, a store on its own is dead code that some compilers drop.
}

UNIT_TEST_FIXTURE(BlackboxTest, StartedAndCleanExitLinesAreWritten) {
    EXPECT_EXIT({
        {
            Blackbox blackbox(logPath);
        }
        std::exit(0);
    }, testing::ExitedWithCode(0), "");

    std::vector<std::string> lines = logLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_TRUE(lines[0].starts_with("--- started "));
    EXPECT_CONTAINS(lines[0], "pid=");
    EXPECT_TRUE(lines[1].starts_with("--- clean exit "));
    EXPECT_FALSE(std::filesystem::exists(oldLogPath.toStdPath())); // A fresh file has nothing to rotate.
}

UNIT_TEST_FIXTURE(BlackboxTest, ExitDuringUnwindingIsNotClean) {
    // The destructor runs during stack unwinding too, and calling that a clean exit would send whoever reads
    // the log looking in the wrong place.
    EXPECT_EXIT({
        try {
            Blackbox blackbox(logPath);
            throw std::runtime_error("unwinding");
        } catch (const std::exception &) {
        }
        std::exit(0);
    }, testing::ExitedWithCode(0), "");

    std::vector<std::string> lines = logLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_TRUE(lines[1].starts_with("--- exiting with exception "));
}

UNIT_TEST_FIXTURE(BlackboxTest, CrashIsInTheFileBeforeTheChainedCallbackRuns) {
    // The chained callback is the one that may never return - a crash dialog, a console wait - so the file has
    // to hold the whole trace by the time that callback sees the final chunk. The callback below checks the
    // file at exactly that moment and reports on stderr, which is what the death test sees.
    EXPECT_DEATH({
        GTEST_FLAG_SET(catch_exceptions, false);

        initStackTraceOnCrash([](std::string_view text, bool final) {
            printCrashChunk(text, final);
            if (final)
                printCrashChunk(Blob::fromFile(logPath).str().contains("blackboxCrashingFunction") ? "trace was in the file" : "trace was not in the file", false);
        });
        Blackbox blackbox(logPath);
        blackboxCrashingFunction();
    }, testing::AllOf(testing::HasSubstr("Crashed because of"), testing::HasSubstr("trace was in the file")));

    std::string log(Blob::fromFile(logPath).str());
    EXPECT_CONTAINS(log, "--- started ");
    EXPECT_CONTAINS(log, "Crashed because of");
    EXPECT_CONTAINS(log, "blackboxCrashingFunction");
    EXPECT_MISSES(log, "exit"); // The process died, no exit line was ever written.
}

UNIT_TEST_FIXTURE(BlackboxTest, OversizedLogIsRotated) {
    FileOutputStream(logPath).write(std::string(1024 * 1024 + 1, 'x'));

    EXPECT_EXIT({
        {
            Blackbox blackbox(logPath);
        }
        std::exit(0);
    }, testing::ExitedWithCode(0), "");

    EXPECT_EQ(std::filesystem::file_size(oldLogPath.toStdPath()), 1024 * 1024 + 1);
    std::vector<std::string> lines = logLines();
    ASSERT_EQ(lines.size(), 2);
    EXPECT_TRUE(lines[0].starts_with("--- started "));
}

#endif // !__ANDROID__
