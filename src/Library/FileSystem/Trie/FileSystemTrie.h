#pragma once

#include <cassert>
#include <utility>
#include <memory>
#include <optional>
#include <unordered_map>
#include <string>

#include "Utility/String/TransparentFunctors.h"

#include "Library/FileSystem/Interface/FileSystemPath.h"

namespace detail {

template<class T>
class FileSystemTrie;

template<class T>
class FileSystemTrieNode {
 public:
    FileSystemTrieNode(FileSystemTrieNode *parent, std::string_view key) : _parent(parent), _key(key) {}

    [[nodiscard]] bool hasValue() const {
        return _value.has_value();
    }

    [[nodiscard]] T &value() {
        return *_value;
    }

    [[nodiscard]] const T &value() const {
        return *_value;
    }

    [[nodiscard]] const auto &children() const {
        return _children;
    }

    [[nodiscard]] FileSystemTrieNode *child(std::string_view name) const {
        auto pos = _children.find(name);
        return pos == _children.end() ? nullptr : pos->second.get();
    }

    [[nodiscard]] FileSystemTrieNode *parent() const {
        return _parent;
    }

 private:
    friend class FileSystemTrie<T>;

 private:
    /** Parent of this node. */
    FileSystemTrieNode *_parent = nullptr;

    /** Key in the parent node. It's only used for node removal, and is not exposed through getters. */
    std::string _key;

    /** Value, if any. */
    std::optional<T> _value;

    /** Children map. It's possible to jump through hoops here and use `std::unordered_set` of 
     * `std::unique_ptr<FileSystemTrieNode>` since the key is stored in the `FileSystemTrieNode` anyway. 
     * But it's just not worth it. */
    std::unordered_map<TransparentString, std::unique_ptr<FileSystemTrieNode>, TransparentStringHash, TransparentStringEquals> _children;
};

/**
 * Trie map from `FileSystemPath` to `T`.
 * 
 * Each node can contain a value, even if it's not a leaf node. If the user needs a trie that only contains values in
 * the leaf nodes, then it's up to the user to maintain this invariant.
 * 
 * The only invariant maintained by `FileSystemTrie` is that it automatically drops all nodes that don't lie on a path 
 * to a value node. This means, for example, that the following code will leave the trie empty:
 * ```
 * FileSystemTrie<bool> trie;
 * trie.insertOrAssign(FileSystemPath("a/b/c"), true);
 * trie.erase(FileSystemPath("a/b"));
 * // trie is now empty, trie.find(FileSystemPath("a")) will return nullptr.
 * ```
 * 
 * Note that the interface of `FileSystemTrie` is a bit different from what one would expect a map-like class to offer.
 * We are directly exposing the fact that `FileSystemTrie` is a tree. For example, `erase` removes subtrees. A more
 * STL-like interface would expose something like `equal_range` to get a subtree, and a two-arg `erase` to drop it,
 * but that's just adding complexity where it's not warranted.
 */
template<class T>
class FileSystemTrie {
 public:
    using Node = FileSystemTrieNode<T>;

    FileSystemTrie() : _root(std::make_unique<Node>(nullptr, "")) {}

    Node *root() {
        return _root.get();
    }

    const Node *root() const {
        return const_cast<FileSystemTrie *>(this)->root();
    }

    Node *find(Node *base, FileSystemPathView relativePath) {
        assert(base);

        for (std::string_view chunk : relativePath.split()) {
            base = base->child(chunk);
            if (!base)
                return base;
        }

        return base;
    }

    Node *find(FileSystemPathView path) {
        return find(root(), path);
    }

    const Node *find(const Node *base, FileSystemPathView relativePath) const {
        return const_cast<FileSystemTrie *>(this)->find(base, relativePath);
    }

    const Node *find(FileSystemPathView path) const {
        return const_cast<FileSystemTrie *>(this)->find(path);
    }

    Node *walk(Node *base, FileSystemPathView relativePath, FileSystemPathView *tail = nullptr) {
        assert(base);

        for (std::string_view chunk : relativePath.split()) {
            if (Node *child = base->child(chunk)) {
                base = child;
            } else {
                if (tail)
                    *tail = relativePath.split().tailAt(chunk);
                return base;
            }
        }

        if (tail)
            *tail = {};
        return base;
    }

    Node *walk(FileSystemPathView path, FileSystemPathView *tail = nullptr) {
        return walk(root(), path, tail);
    }

    const Node *walk(const Node *base, FileSystemPathView relativePath, FileSystemPathView *tail = nullptr) const {
        return const_cast<FileSystemTrie *>(this)->walk(base, relativePath, tail);
    }

    const Node *walk(FileSystemPathView path, FileSystemPathView *tail = nullptr) const {
        return const_cast<FileSystemTrie *>(this)->walk(path, tail);
    }

    bool erase(Node *base, FileSystemPathView relativePath = {}) {
        assert(base);

        base = find(base, relativePath);
        if (!base)
            return false;

        base->_children.clear();
        base->_value = std::nullopt;
        _prune(base);
        return true;
    }

    bool erase(FileSystemPathView path) {
        return erase(root(), path);
    }

    void chop(Node *base, FileSystemPathView relativePath = {}) {
        assert(base);

        base = find(base, relativePath);
        if (!base)
            return;

        base->_children.clear();
        _prune(base);
    }

    void chop(FileSystemPathView path) {
        chop(root(), path);
    }

    Node *insertOrAssign(Node *base, FileSystemPathView relativePath, T value) {
        assert(base);

        base = _grow(base, relativePath);
        base->_value = std::move(value);
        return base;
    }

    Node *insertOrAssign(FileSystemPathView path, T value) {
        return insertOrAssign(root(), path, std::move(value));
    }

    std::unique_ptr<Node> extract(Node *node) {
        assert(node);

        Node *parent = node->_parent;
        if (parent) {
            auto pos = parent->_children.find(node->_key);

            std::unique_ptr<Node> result = std::move(pos->second);
            result->_parent = nullptr;
            result->_key.clear();

            parent->_children.erase(pos);
            _prune(parent); // parent might have become empty as a result.

            return result;
        } else {
            // Extracting root node.
            std::unique_ptr<Node> result = std::make_unique<Node>(nullptr, "");
            swap(result, _root);
            return result;
        }
    }

    Node *insertOrAssign(Node *base, FileSystemPathView relativePath, std::unique_ptr<Node> node) {
        assert(base);
        assert(node);
        assert(node->_parent == nullptr);
        assert(node->_key.empty());

        if (base == root() && relativePath.isEmpty()) { // Replacing root.
            _root = std::move(node);
            return root();
        }

        if (!node->hasValue() && node->children().empty()) { // Replacing with an empty node == node deletion.
            erase(base, relativePath);
            return nullptr;
        }

        // We need to preserve pointer values so will have to jump through hoops here a bit.
        Node *branch = _grow(base, relativePath);
        node->_parent = branch->_parent;
        node->_key = std::move(branch->_key); // Can move the string out as branch will be destroyed in the next statement.
        return (node->_parent->_children[node->_key] = std::move(node)).get();
    }

    Node *insertOrAssign(FileSystemPathView path, std::unique_ptr<Node> node) {
        return insertOrAssign(root(), path, std::move(node));
    }

    void clear() {
        erase(root());
    }

    bool isEmpty() const {
        return !root()->hasValue() && root()->children().empty();
    }

 private:
    Node *_prune(Node *node) {
        assert(node);
        while (node->children().empty() && !node->hasValue() && node != root()) {
            Node *parent = node->_parent;
            parent->_children.erase(node->_key); // node is destroyed here.
            node = parent;
        }
        return node;
    }

    Node *_grow(Node *base, const FileSystemPathView relativePath) {
        assert(base);

        for (std::string_view chunk : relativePath.split()) {
            if (Node *child = base->child(chunk)) {
                base = child;
                continue;
            }

            base = base->_children.emplace(chunk, std::make_unique<Node>(base, chunk)).first->second.get();
        }

        return base;
    }

 private:
    std::unique_ptr<FileSystemTrieNode<T>> _root;
};

} // namespace detail
