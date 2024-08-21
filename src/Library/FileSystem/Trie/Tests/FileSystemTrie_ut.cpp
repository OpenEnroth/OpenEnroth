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

UNIT_TEST(FileSystemTrie, ExtractInsert) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie;
    FileSystemTrieNode<int> *nodea = trie.insertOrAssign(FileSystemPath("a"), 10);

    std::unique_ptr<FileSystemTrieNode<int>> extracted = trie.extract(nodea);
    EXPECT_EQ(extracted->parent(), nullptr);
    EXPECT_EQ(trie.find(FileSystemPath("a")), nullptr);

    FileSystemTrieNode<int> *nodeb = trie.insertOrAssign(FileSystemPath("b"), std::move(extracted));
    EXPECT_EQ(nodea, nodeb); // Checking that pointers are preserved.
    EXPECT_EQ(nodeb->parent(), trie.root());

    EXPECT_TRUE(trie.erase(nodeb));
    EXPECT_TRUE(trie.isEmpty());
}

UNIT_TEST(FileSystemTrie, ExtractInsertRoot) {
    using namespace detail; // NOLINT

    FileSystemTrie<int> trie0;
    FileSystemTrieNode<int> *node = trie0.insertOrAssign(FileSystemPath("a"), 10);
    FileSystemTrieNode<int> *root0 = trie0.root();

    FileSystemTrie<int> trie1;
    trie1.insertOrAssign(FileSystemPath(), trie0.extract(root0));

    EXPECT_NE(trie0.root(), nullptr); // New root was created.
    EXPECT_TRUE(trie0.isEmpty());
    EXPECT_FALSE(trie1.isEmpty());
    EXPECT_EQ(trie1.root(), root0);
    EXPECT_EQ(trie1.find(FileSystemPath("a")), node);
}
