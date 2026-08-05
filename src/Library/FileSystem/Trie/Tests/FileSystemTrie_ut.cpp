#include <memory>
#include <utility>

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
    FileSystemPathView tail;
    FileSystemPath path("a/b/c");
    FileSystemTrieNode<bool> *node = trie.walk(path, &tail);

    EXPECT_EQ(node, trie.root());
    EXPECT_EQ(tail, FileSystemPath("a/b/c"));
}

UNIT_TEST(FileSystemTrie, NonEmptyWalk) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    trie.insertOrAssign(FileSystemPath("a/b"), 10);

    FileSystemPathView tail;
    FileSystemTrieNode<int> *node = nullptr;

    FileSystemPath path0("a/b/c");
    node = trie.walk(path0, &tail);
    EXPECT_EQ(node, trie.find(FileSystemPath("a/b")));
    EXPECT_EQ(tail, FileSystemPath("c"));

    FileSystemPath path1("a/b");
    node = trie.walk(path1, &tail);
    EXPECT_EQ(node, trie.find(FileSystemPath("a/b")));
    EXPECT_EQ(tail, FileSystemPath());

    FileSystemPath path2("");
    node = trie.walk(path2, &tail);
    EXPECT_EQ(node, trie.root());
    EXPECT_EQ(tail, FileSystemPath());
}

UNIT_TEST(FileSystemTrie, WalkClearsTail) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    trie.insertOrAssign(FileSystemPath("a/b"), 10);

    FileSystemPath path0("a/b");
    FileSystemPathView tail0 = path0;
    trie.walk(path0, &tail0);
    EXPECT_EQ(tail0, FileSystemPathView());

    FileSystemPath path1("");
    FileSystemPathView tail1 = path0;
    trie.walk(path1, &tail1);
    EXPECT_EQ(tail1, FileSystemPath());
}

