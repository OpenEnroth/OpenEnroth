#pragma once

namespace detail {
template<class FileSystem>
class FileSystemForwardingPtr;
template<class FileSystem>
class FileSystemForwardingPtrBase;
template<class T>
class FileSystemTrieNode;
template<class T>
class FileSystemTrie;
} // namespace detail

class FileSystem;
class FileSystemPath;
class FileSystemPathView;
