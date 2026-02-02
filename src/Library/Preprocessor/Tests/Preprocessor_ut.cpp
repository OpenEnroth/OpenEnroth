#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Memory/MemoryFileSystem.h"
#include "Library/Preprocessor/Preprocessor.h"

UNIT_TEST(Preprocessor, NoIncludes) {
    MemoryFileSystem fs("memfs");
    Blob source = Blob::fromString("int x = 1;").withDisplayPath("test.glsl");

    Blob result = pp::preprocess(source, &fs);

    EXPECT_CONTAINS(result.str(), "int x = 1");
}

UNIT_TEST(Preprocessor, SimpleInclude) {
    MemoryFileSystem fs("memfs");
    fs.write("common.glsl", Blob::fromString("float PI = 3.14;"));
    Blob source = Blob::fromString("#include \"common.glsl\"\nint x = 1;").withDisplayPath("test.glsl");

    Blob result = pp::preprocess(source, &fs);

    EXPECT_CONTAINS(result.str(), "float PI = 3.14");
    EXPECT_CONTAINS(result.str(), "int x = 1");
}

UNIT_TEST(Preprocessor, DefineAndIfdef) {
    MemoryFileSystem fs("memfs");
    Blob source = Blob::fromString("#define FOO\n#ifdef FOO\nint x = 1;\n#endif").withDisplayPath("test.glsl");

    Blob result = pp::preprocess(source, &fs);

    EXPECT_CONTAINS(result.str(), "int x = 1");
}

UNIT_TEST(Preprocessor, IfdefNotDefined) {
    MemoryFileSystem fs("memfs");
    Blob source = Blob::fromString("#ifdef FOO\nint x = 1;\n#endif\nint y = 2;").withDisplayPath("test.glsl");

    Blob result = pp::preprocess(source, &fs);

    EXPECT_MISSES(result.str(), "int x = 1");
    EXPECT_CONTAINS(result.str(), "int y = 2");
}

UNIT_TEST(Preprocessor, IncludeNotFound) {
    MemoryFileSystem fs("memfs");
    Blob source = Blob::fromString("#include \"missing.glsl\"").withDisplayPath("test.glsl");

    EXPECT_THROW(pp::preprocess(source, &fs), std::runtime_error);
}

UNIT_TEST(Preprocessor, NestedInclude) {
    MemoryFileSystem fs("memfs");
    fs.write("a.glsl", Blob::fromString("#include \"b.glsl\"\nint a = 1;"));
    fs.write("b.glsl", Blob::fromString("int b = 2;"));
    Blob source = Blob::fromString("#include \"a.glsl\"").withDisplayPath("test.glsl");

    Blob result = pp::preprocess(source, &fs);

    EXPECT_CONTAINS(result.str(), "int a = 1");
    EXPECT_CONTAINS(result.str(), "int b = 2");
}

UNIT_TEST(Preprocessor, Preamble) {
    MemoryFileSystem fs("memfs");
    Blob source = Blob::fromString("#ifdef GL_ES\nint x = 1;\n#endif").withDisplayPath("test.glsl");

    // Without preamble, GL_ES is not defined.
    Blob resultWithout = pp::preprocess(source, &fs);
    EXPECT_MISSES(resultWithout.str(), "int x = 1");

    // With preamble defining GL_ES.
    Blob resultWith = pp::preprocess(source, &fs, "#define GL_ES\n");
    EXPECT_CONTAINS(resultWith.str(), "int x = 1");
}

UNIT_TEST(Preprocessor, GlslDirectivesPassThrough) {
    // GLSL-specific directives like #version and #extension should pass through unchanged
    // when registered as passthrough directives.
    MemoryFileSystem fs("memfs");
    Blob source = Blob::fromString("#version 410 core\n#extension GL_ARB_foo : enable\nvoid main() {}").withDisplayPath("test.glsl");

    static constexpr std::string_view directives[] = {"version", "extension"};
    Blob result = pp::preprocess(source, &fs, {}, directives);

    EXPECT_CONTAINS(result.str(), "#version 410 core");
    EXPECT_CONTAINS(result.str(), "#extension GL_ARB_foo : enable");
    EXPECT_CONTAINS(result.str(), "void main()");
}
