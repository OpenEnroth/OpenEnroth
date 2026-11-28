#include "FileSystemDump.h"

#include <iomanip>
#include <ranges>
#include <utility>
#include <string>
#include <vector>

class FileSystemDumper {
 public:
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
    void dump(PathView path) {
        writeOutDir(path);
        if (_entries == _maxEntries)
            return;

        std::vector<DirectoryEntry> entries = _fs->ls(path);
        std::ranges::sort(entries);

        for (const DirectoryEntry &entry : entries) {
            Path entryPath = path / entry.name;

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

    void writeOutDir(PathView path) {
        _target->push_back(FileSystemDumpEntry(path.str(), FILE_DIRECTORY));

        _entries++;
        assert(_entries <= _maxEntries);
    }

    void writeOutFile(PathView path) {
        Blob content;
        if (_flags & FILE_SYSTEM_DUMP_WITH_CONTENTS)
            content = _fs->read(path);

        _target->push_back(FileSystemDumpEntry(path.str(), FILE_REGULAR, std::move(content)));

        _entries++;
        assert(_entries <= _maxEntries);
    }

 private:
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

void PrintTo(const FileSystemDumpEntry &entry, std::ostream *stream) {
    *stream << std::quoted(entry.path);
    if (entry.type == FILE_REGULAR)
        *stream << ": " << std::quoted(entry.content.str());
}
