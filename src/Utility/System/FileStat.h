#pragma once

#include <compare>
#include <cstdint>
#include <string>
#include <utility>

enum class FileType {
    FILE_INVALID, // Returned by `stat` calls if file doesn't exist.
    FILE_REGULAR,
    FILE_DIRECTORY,
};
using enum FileType;

struct FileStat {
    FileStat() = default;
    FileStat(FileType type, std::int64_t size) : type(type), size(size) {}

    FileType type = FILE_INVALID; // Invalid means file doesn't exist.
    std::int64_t size = 0; // Always zero for directories.

    explicit operator bool() const {
        return type != FILE_INVALID;
    }

    friend bool operator==(const FileStat &l, const FileStat &r) = default;
};

struct DirectoryEntry {
    DirectoryEntry() = default;
    DirectoryEntry(std::string name, FileType type) : name(std::move(name)), type(type) {}

    std::string name;
    FileType type = FILE_INVALID; // Never invalid when returned from `ls` calls.

    // Make entries sortable. If two entries have the same name, then files go before directories.
    friend std::strong_ordering operator<=>(const DirectoryEntry &l, const DirectoryEntry &r) = default;
};
