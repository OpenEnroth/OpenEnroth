#include <cstdlib>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"

UNIT_TEST(FileOutputStream, Write) {
    const char* tmpfile = "tmp_test.txt";
    const char* tmpfilecontent = "1234\n";
    size_t tmpfilesize = strlen(tmpfilecontent);

    FileOutputStream out(tmpfile);
    out.Write(tmpfilecontent, tmpfilesize);
    out.Close();

    FileInputStream in(tmpfile);

    char buf[1024] = {};
    size_t bytes = in.Read(buf, 1024);;
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(strcmp(buf, tmpfilecontent), 0);

    bytes = in.Read(buf, 1024);
    EXPECT_EQ(bytes, 0);
    in.Close();

    remove(tmpfile);
}

UNIT_TEST(FileInputStream, Skip) {
    const char* tmpfile = "tmp_test.txt";
    std::string data(3000, 'a');

    FileOutputStream out(tmpfile);
    out.Write(data.data(), data.size());
    out.Close();

    FileInputStream in(tmpfile);
    size_t bytes = in.Skip(50);
    EXPECT_EQ(bytes, 50);

    bytes = in.Skip(2000);
    EXPECT_EQ(bytes, 2000);

    char buf[1024] = {};
    bytes = in.Read(buf, 1024);
    EXPECT_EQ(bytes, 950);
    EXPECT_EQ(std::string_view(buf, 950), std::string(950, 'a'));
    in.Close();

    remove(tmpfile);
}
