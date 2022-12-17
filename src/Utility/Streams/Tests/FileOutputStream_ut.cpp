#include <cstdlib>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"

UNIT_TEST(FileOutputStream, Write) {
    const char* tmpfile = "tmp_test.txt";
    const char* tmpfilecontent = "1234\n";
    size_t tmpfilesize = strlen(tmpfilecontent);

    FileOutputStream out(tmpfile);
    out.Write(tmpfilecontent, tmpfilesize);
    out.Close();

    FILE* in = fopen(tmpfile, "rb");
    EXPECT_NE(in, nullptr);

    char buf[1024] = {};
    size_t bytes = fread(buf, 1, 1024, in);
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(strcmp(buf, tmpfilecontent), 0);

    bytes = fread(buf, 1, 1024, in);
    EXPECT_EQ(bytes, 0);
    EXPECT_TRUE(feof(in));

    fclose(in);
    remove(tmpfile);
}
