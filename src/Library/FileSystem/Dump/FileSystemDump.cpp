#include "FileSystemDump.h"

#include <cstdio>
#include <iomanip>
#include <ranges>
#include <utility>
#include <string>
#include <vector>

#include "Utility/String/Ascii.h"
#include "Utility/String/Format.h"

class FileSystemDumper {
 public:
    FileSystemDumper(FILE *stream, FileSystem *fs, FileSystemDumpFlags flags, int maxEntries) : _stream(stream), _fs(fs), _flags(flags), _maxEntries(maxEntries) {
        assert(stream);
        assert(fs);
    }

    FileSystemDumper(std::vector<FileSystemDumpEntry> *target, FileSystem *fs, FileSystemDumpFlags flags, int maxEntries) : _target(target), _fs(fs), _flags(flags), _maxEntries(maxEntries) {
        assert(target);
        assert(fs);
    }

    void dump() {
        if (_maxEntries == 0)
            return;

        dump({});
    }

 private:
    void dump(FileSystemPathView path) {
        writeOutDir(path);
        if (_entries == _maxEntries)
            return;

        std::vector<DirectoryEntry> entries = _fs->ls(path);
        std::ranges::sort(entries);

        for (const DirectoryEntry &entry : entries) {
            FileSystemPath entryPath = path / entry.name;

            if (entry.type == FILE_REGULAR) {
                writeOutFile(entryPath);
            } else {
                assert(entry.type == FILE_DIRECTORY);
                dump(entryPath);
            }

            if (_entries == _maxEntries)
                return;
        }
    }

    void writeOutDir(FileSystemPathView path) {
        if (_target) {
            _target->push_back(FileSystemDumpEntry(path.string(), FILE_DIRECTORY));
        } else {
            fmt::println(_stream, "{}", path.string());
        }

        _entries++;
        assert(_entries <= _maxEntries);
    }

    void writeOutFile(FileSystemPathView path) {
        Blob content;
        if (_flags & FILE_SYSTEM_DUMP_WITH_CONTENTS)
            content = _fs->read(path);

        if (_target) {
            _target->push_back(FileSystemDumpEntry(path.string(), FILE_REGULAR, std::move(content)));
        } else {
            if (_flags & FILE_SYSTEM_DUMP_WITH_CONTENTS) {
                fmt::println(_stream, "{}: \"{}\" ", path.string(), ascii::toPrintable(content.string_view(), '_'));
            } else {
                fmt::println(_stream, "{}", path.string());
            }
        }

        _entries++;
        assert(_entries <= _maxEntries);
    }

 private:
    FILE *_stream = nullptr;
    std::vector<FileSystemDumpEntry> *_target = nullptr;
    FileSystem *_fs = nullptr;
    FileSystemDumpFlags _flags = 0;
    int _maxEntries = 0;
    int _entries = 0;
};

std::vector<FileSystemDumpEntry> dumpFileSystem(FileSystem *fs, FileSystemDumpFlags flags, int maxEntries) {
    std::vector<FileSystemDumpEntry> result;
    FileSystemDumper(&result, fs, flags, maxEntries).dump();
    return result;
}

void dumpFileSystem(FILE *stream, FileSystem *fs, FileSystemDumpFlags flags, int maxEntries) {
    FileSystemDumper(stream, fs, flags, maxEntries).dump();
}

void PrintTo(const FileSystemDumpEntry &entry, std::ostream *stream) {
    // Symmetric to `FileSystemDumper::writeOut*`.
    *stream << std::quoted(entry.path);
    if (entry.type == FILE_REGULAR)
        *stream << ": " << std::quoted(entry.content.string_view());
}
