#include <cstdlib>
#include <string>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/Streams/FileInputStream.h"
#include "Utility/Testing/TestNonExistingFile.h"

UNIT_TEST(FileOutputStream, Write) {
    const char *tmpfile = "tmp_test.txt";
    const char *tmpfilecontent = "1234\n";
    size_t tmpfilesize = strlen(tmpfilecontent);

    TestNonExistingFile tmp(tmpfile);

    FileOutputStream out(tmpfile);
    out.write(tmpfilecontent, tmpfilesize);
    out.close();

    FileInputStream in(tmpfile);

    char buf[1024] = {};
    size_t bytes = in.read(buf, 1024);;
    EXPECT_EQ(bytes, 5);
    EXPECT_EQ(strcmp(buf, tmpfilecontent), 0);

    bytes = in.read(buf, 1024);
    EXPECT_EQ(bytes, 0);
    in.close();
}

UNIT_TEST(FileOutputStream, WriteZero) {
    const char *tmpfile = "tmp_test.txt";

    TestNonExistingFile tmp(tmpfile);

    FileOutputStream out(tmpfile);
    EXPECT_NO_THROW(out.write(nullptr, 0));
    out.close();

    FileInputStream in(tmpfile);
    EXPECT_EQ(in.readAll(), "");
}
