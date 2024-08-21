#pragma once

#include "ProxyFileSystem.h"

class ScopedFileSystemSwizzle {
 public:
    ScopedFileSystemSwizzle(ProxyFileSystem *proxy, FileSystem *base) {
        _proxy = proxy;
        _base = proxy->base();
        _proxy->setBase(base);
    }

    ~ScopedFileSystemSwizzle() {
        _proxy->setBase(_base);
    }

 private:
    ProxyFileSystem *_proxy = nullptr;
    FileSystem *_base = nullptr;
};
