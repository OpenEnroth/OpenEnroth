#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobInputStream.h"

UNIT_TEST(BlobInputStream, MixedBlobAndNonBlobReads) {
    Blob blob = Blob::fromString("HelloWorldFoo");
    BlobInputStream stream(std::move(blob));

    Blob hello = stream.readAsBlob(5);
    EXPECT_EQ(hello.str(), "Hello");

    char buf[5];
    EXPECT_EQ(stream.read(buf, 5), 5u);
    EXPECT_EQ(std::string_view(buf, 5), "World");

    Blob rest = stream.readAllAsBlob();
    EXPECT_EQ(rest.str(), "Foo");

    EXPECT_EQ(stream.read(buf, 1), 0u); // Stream exhausted.
}

UNIT_TEST(BlobInputStream, SkipThenReadAsBlob) {
    Blob blob = Blob::fromString("HeaderPayload");
    BlobInputStream stream(std::move(blob));

    EXPECT_EQ(stream.skip(6), 6u);

    Blob payload = stream.readAsBlobOrFail(7);
    EXPECT_EQ(payload.str(), "Payload");
}

UNIT_TEST(BlobInputStream, ReadAsBlobThenReadAll) {
    Blob blob = Blob::fromString("PrefixSuffix");
    BlobInputStream stream(std::move(blob));

    Blob prefix = stream.readAsBlob(6);
    EXPECT_EQ(prefix.str(), "Prefix");

    std::string rest = stream.readAll();
    EXPECT_EQ(rest, "Suffix");
}

UNIT_TEST(BlobInputStream, ReadAllAsBlobEmpty) {
    Blob blob = Blob::fromString("");
    BlobInputStream stream(std::move(blob));

    Blob result = stream.readAllAsBlob();
    EXPECT_EQ(result.size(), 0u);
}

UNIT_TEST(BlobInputStream, ReadAllAsBlobAfterExhausting) {
    Blob blob = Blob::fromString("abc");
    BlobInputStream stream(std::move(blob));

    EXPECT_EQ(stream.skip(3), 3u);
    Blob result = stream.readAllAsBlob();
    EXPECT_EQ(result.size(), 0u);
}

UNIT_TEST(BlobInputStream, ReadAsBlobOrFailThrows) {
    Blob blob = Blob::fromString("short");
    BlobInputStream stream(std::move(blob));

    EXPECT_THROW_MESSAGE((void) stream.readAsBlobOrFail(100), "100");
}

UNIT_TEST(BlobInputStream, ReadAsBlobShort) {
    Blob blob = Blob::fromString("abc");
    BlobInputStream stream(std::move(blob));

    Blob result = stream.readAsBlob(100);
    EXPECT_EQ(result.str(), "abc");
}

UNIT_TEST(BlobInputStream, CloseIdempotent) {
    Blob blob = Blob::fromString("hello");
    BlobInputStream stream(std::move(blob));
    stream.close();
    EXPECT_FALSE(stream.isOpen());
    EXPECT_NO_THROW(stream.close()); // Double close is fine.
    EXPECT_FALSE(stream.isOpen());
}

UNIT_TEST(BlobInputStream, ReopenAfterClose) {
    Blob blob1 = Blob::fromString("first");
    Blob blob2 = Blob::fromString("second");

    BlobInputStream stream(std::move(blob1));
    EXPECT_EQ(stream.readAll(), "first");
    stream.close();

    stream.open(std::move(blob2));
    EXPECT_TRUE(stream.isOpen());
    EXPECT_EQ(stream.readAll(), "second");
}

UNIT_TEST(BlobInputStream, SizeMatchesBlobSize) {
    Blob blob = Blob::fromString("hello world");
    BlobInputStream stream(std::move(blob));
    EXPECT_EQ(stream.size(), 11u);
}

UNIT_TEST(BlobInputStream, PositionStartsAtZero) {
    Blob blob = Blob::fromString("hello");
    BlobInputStream stream(std::move(blob));
    EXPECT_EQ(stream.position(), 0u);
}

UNIT_TEST(BlobInputStream, PositionAdvancesOnRead) {
    Blob blob = Blob::fromString("hello world");
    BlobInputStream stream(std::move(blob));

    Blob hello = stream.readAsBlob(5);
    EXPECT_EQ(stream.position(), 5u);

    char buf[3];
    stream.readOrFail(buf, 3);
    EXPECT_EQ(stream.position(), 8u);
}

UNIT_TEST(BlobInputStream, PositionAfterReadAllAsBlob) {
    Blob blob = Blob::fromString("data");
    BlobInputStream stream(std::move(blob));

    (void) stream.readAllAsBlob();
    EXPECT_EQ(stream.position(), 4u);
    EXPECT_EQ(stream.position(), stream.size());
}

UNIT_TEST(BlobInputStream, PositionResetsOnReopen) {
    Blob blob1 = Blob::fromString("first");
    Blob blob2 = Blob::fromString("second!");

    BlobInputStream stream(std::move(blob1));
    (void) stream.skip(3);
    EXPECT_EQ(stream.position(), 3u);

    stream.close();
    stream.open(std::move(blob2));
    EXPECT_EQ(stream.position(), 0u);
    EXPECT_EQ(stream.size(), 7u);
}

