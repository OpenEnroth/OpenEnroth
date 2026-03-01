#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobInputStream.h"

UNIT_TEST(BlobInputStream, MixedBlobAndNonBlobReads) {
    Blob blob = Blob::fromString("HelloWorldFoo");
    BlobInputStream stream(std::move(blob));

    // Read "Hello" as a subblob.
    Blob hello = stream.readAsBlob(5);
    EXPECT_EQ(hello.str(), "Hello");

    // Read "World" via non-blob read.
    char buf[5];
    EXPECT_EQ(stream.read(buf, 5), 5u);
    EXPECT_EQ(std::string_view(buf, 5), "World");

    // Read remaining as a subblob.
    Blob rest = stream.readAllAsBlob();
    EXPECT_EQ(rest.str(), "Foo");

    // Stream should be exhausted.
    EXPECT_EQ(stream.read(buf, 1), 0u);
}

UNIT_TEST(BlobInputStream, SkipThenReadAsBlob) {
    Blob blob = Blob::fromString("HeaderPayload");
    BlobInputStream stream(std::move(blob));

    // Skip the header.
    EXPECT_EQ(stream.skip(6), 6u);

    // Read payload as a subblob.
    Blob payload = stream.readAsBlobOrFail(7);
    EXPECT_EQ(payload.str(), "Payload");
}

UNIT_TEST(BlobInputStream, ReadAsBlobThenReadAll) {
    Blob blob = Blob::fromString("PrefixSuffix");
    BlobInputStream stream(std::move(blob));

    // Read prefix as a subblob.
    Blob prefix = stream.readAsBlob(6);
    EXPECT_EQ(prefix.str(), "Prefix");

    // Read remaining via non-blob readAll.
    std::string rest = stream.readAll();
    EXPECT_EQ(rest, "Suffix");
}

UNIT_TEST(BlobInputStream, ReadUntilDelimiterFound) {
    Blob blob = Blob::fromString(std::string("hello\0world", 11));
    BlobInputStream input(std::move(blob));
    EXPECT_EQ(input.readUntil('\0'), "hello");
    EXPECT_EQ(input.readAll(), "world");
}
