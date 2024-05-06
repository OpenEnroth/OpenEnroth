#include "Testing/Unit/UnitTest.h"

#include "Library/FileSystem/Trie/FileSystemTrie.h"

UNIT_TEST(FileSystemTrie, AddRemove) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;

    trie.insertOrAssign(FileSystemPath("a/b/c"), 10);
    trie.insertOrAssign(FileSystemPath("a/b/e/f/g"), 20);
    EXPECT_TRUE(trie.erase(FileSystemPath("a/b/e/f")));
    EXPECT_FALSE(trie.erase(FileSystemPath("a/b/e/f")));

    EXPECT_NE(trie.find(FileSystemPath("a")), nullptr);
    EXPECT_FALSE(trie.find(FileSystemPath("a"))->hasValue());

    EXPECT_NE(trie.find(FileSystemPath("a/b")), nullptr);
    EXPECT_FALSE(trie.find(FileSystemPath("a/b"))->hasValue());

    EXPECT_NE(trie.find(FileSystemPath("a/b/c")), nullptr);
    EXPECT_TRUE(trie.find(FileSystemPath("a/b/c"))->hasValue());
    EXPECT_EQ(trie.find(FileSystemPath("a/b/c"))->value(), 10);

    EXPECT_EQ(trie.find(FileSystemPath("a/b/e")), nullptr);

    EXPECT_TRUE(trie.erase(FileSystemPath("a/b/c")));
    EXPECT_FALSE(trie.erase(FileSystemPath("a/b/c")));
    EXPECT_EQ(trie.find(FileSystemPath("a")), nullptr);
}

UNIT_TEST(FileSystemTrie, EmptyWalk) {
    using namespace detail; // NOLINT

    FileSystemTrie<bool> trie;
    FileSystemPath tail;
    FileSystemTrieNode<bool> *node = trie.walk(FileSystemPath("a/b/c"), &tail);

    EXPECT_EQ(node, trie.root());
    EXPECT_EQ(tail, FileSystemPath("a/b/c"));
}

UNIT_TEST(FileSystemTrie, NonEmptyWalk) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    trie.insertOrAssign(FileSystemPath("a/b"), 10);

    FileSystemPath tail;
    FileSystemTrieNode<int> *node = nullptr;

    node = trie.walk(FileSystemPath("a/b/c"), &tail);
    EXPECT_EQ(node, trie.find(FileSystemPath("a/b")));
    EXPECT_EQ(tail, FileSystemPath("c"));

    node = trie.walk(FileSystemPath("a/b"), &tail);
    EXPECT_EQ(node, trie.find(FileSystemPath("a/b")));
    EXPECT_EQ(tail, FileSystemPath());

    node = trie.walk(FileSystemPath(""), &tail);
    EXPECT_EQ(node, trie.root());
    EXPECT_EQ(tail, FileSystemPath());
}

UNIT_TEST(FileSystemTrie, WalkClearsTail) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    trie.insertOrAssign(FileSystemPath("a/b"), 10);

    FileSystemPath tail0("a");
    trie.walk(FileSystemPath("a/b"), &tail0);
    EXPECT_EQ(tail0, FileSystemPath());

    FileSystemPath tail1("a");
    trie.walk(FileSystemPath(""), &tail1);
    EXPECT_EQ(tail1, FileSystemPath());
}
