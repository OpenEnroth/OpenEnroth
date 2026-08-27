#include <memory>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Trie/FileSystemTrie.h"

UNIT_TEST(FileSystemTrie, AddRemove) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;

    trie.insertOrAssign(Path("a/b/c"), 10);
    trie.insertOrAssign(Path("a/b/e/f/g"), 20);
    EXPECT_TRUE(trie.erase(Path("a/b/e/f")));
    EXPECT_FALSE(trie.erase(Path("a/b/e/f")));

    EXPECT_NE(trie.find(Path("a")), nullptr);
    EXPECT_FALSE(trie.find(Path("a"))->hasValue());

    EXPECT_NE(trie.find(Path("a/b")), nullptr);
    EXPECT_FALSE(trie.find(Path("a/b"))->hasValue());

    EXPECT_NE(trie.find(Path("a/b/c")), nullptr);
    EXPECT_TRUE(trie.find(Path("a/b/c"))->hasValue());
    EXPECT_EQ(trie.find(Path("a/b/c"))->value(), 10);

    EXPECT_EQ(trie.find(Path("a/b/e")), nullptr);

    EXPECT_TRUE(trie.erase(Path("a/b/c")));
    EXPECT_FALSE(trie.erase(Path("a/b/c")));
    EXPECT_EQ(trie.find(Path("a")), nullptr);
}

UNIT_TEST(FileSystemTrie, EmptyWalk) {
    using namespace detail; // NOLINT

    FileSystemTrie<bool> trie;
    PathView tail;
    Path path("a/b/c");
    FileSystemTrieNode<bool> *node = trie.walk(path, &tail);

    EXPECT_EQ(node, trie.root());
    EXPECT_EQ(tail, Path("a/b/c"));
}

UNIT_TEST(FileSystemTrie, NonEmptyWalk) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    trie.insertOrAssign(Path("a/b"), 10);

    PathView tail;
    FileSystemTrieNode<int> *node = nullptr;

    Path path0("a/b/c");
    node = trie.walk(path0, &tail);
    EXPECT_EQ(node, trie.find(Path("a/b")));
    EXPECT_EQ(tail, Path("c"));

    Path path1("a/b");
    node = trie.walk(path1, &tail);
    EXPECT_EQ(node, trie.find(Path("a/b")));
    EXPECT_EQ(tail, Path());

    Path path2("");
    node = trie.walk(path2, &tail);
    EXPECT_EQ(node, trie.root());
    EXPECT_EQ(tail, Path());
}

UNIT_TEST(FileSystemTrie, WalkClearsTail) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    trie.insertOrAssign(Path("a/b"), 10);

    Path path0("a/b");
    PathView tail0 = path0;
    trie.walk(path0, &tail0);
    EXPECT_EQ(tail0, PathView());

    Path path1("");
    PathView tail1 = path0;
    trie.walk(path1, &tail1);
    EXPECT_EQ(tail1, Path());
}

