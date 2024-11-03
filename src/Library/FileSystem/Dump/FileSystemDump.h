#pragma once

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <ostream>
#include <vector>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Flags.h"

enum class FileSystemDumpFlag {
    FILE_SYSTEM_DUMP_WITH_CONTENTS = 1,
};
using enum FileSystemDumpFlag;
MM_DECLARE_FLAGS(FileSystemDumpFlags, FileSystemDumpFlag)

struct FileSystemDumpEntry {
    std::string path;
    FileType type;
    Blob content; // If it's a regular file.

    FileSystemDumpEntry(const FileSystemDumpEntry &other): path(other.path), type(other.type), content(Blob::share(other.content)) {}

    FileSystemDumpEntry(std::string_view path, FileType type, Blob content = {}): path(path), type(type), content(std::move(content)) {
        assert(type == FILE_REGULAR || type == FILE_DIRECTORY);
        assert(content.empty() || type == FILE_REGULAR);
    }

    FileSystemDumpEntry(std::string_view path, FileType type, std::string content) : path(path), type(type) {
        assert(type == FILE_REGULAR || type == FILE_DIRECTORY);
        assert(content.empty() || type == FILE_REGULAR);

        if (!content.empty())
            this->content = Blob::fromString(std::move(content));
    }

    friend bool operator==(const FileSystemDumpEntry &l, const FileSystemDumpEntry &r) {
        return l.path == r.path && l.type == r.type && l.content.string_view() == r.content.string_view();
    }
};

std::vector<FileSystemDumpEntry> dumpFileSystem(FileSystem *fs, FileSystemDumpFlags flags = 0, int maxEntries = 1024 * 1024);

void dumpFileSystem(FILE *stream, FileSystem *fs, FileSystemDumpFlags flags = 0, int maxEntries = 1024 * 1024);

// Google Test support.
void PrintTo(const FileSystemDumpEntry &entry, std::ostream *stream);
