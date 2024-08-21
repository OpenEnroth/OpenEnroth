#include <cstdlib>
#include <string>
#include <filesystem>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"

UNIT_TEST(FileInputStream, Skip) {
    const char *tmpfile = "tmp_test.txt";
    std::string data(3000, 'a');

    ScopedTestFileSlot tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write(data.data(), data.size());
    out.close();

    FileInputStream in(tmpfile);
    size_t bytes = in.skip(50);
    EXPECT_EQ(bytes, 50);

    bytes = in.skip(2000);
    EXPECT_EQ(bytes, 2000);

    char buf[1024] = {};
    bytes = in.read(buf, 1024);
    EXPECT_EQ(bytes, 950);
    EXPECT_EQ(std::string_view(buf, 950), std::string(950, 'a'));
    in.close();
}

UNIT_TEST(FileInputStream, ExceptionMessages) {
    const char *fileName = "afjhrbluxnkskghelxrigjmgdhckeog.txt";

    EXPECT_FALSE(std::filesystem::exists(fileName));
    EXPECT_THROW_MESSAGE(FileInputStream in(fileName), fileName);
}
