#include "RotatingLogSink.h"

#include <cassert>
#include <ranges>
#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/String/Format.h"

RotatingLogSink::RotatingLogSink(std::string_view path, FileSystem *fs, int count): StreamLogSink(openRotatingStream(FileSystemPath(path), fs, count)) {}

std::unique_ptr<OutputStream> RotatingLogSink::openRotatingStream(const FileSystemPath &path, FileSystem *fs, int count) {
    assert(fs);

    auto components = path.components();

    // Find existing log files.
    std::vector<DirectoryEntry> entries;
    if (fs->exists(components.prefix())) {
        entries = fs->ls(components.prefix());
        std::erase_if(entries, [&](const DirectoryEntry &entry) {
            // We're being lazy here and just checking stem & extension. Can do a regex, but that would be an overkill.
            return !entry.name.starts_with(components.stem()) && !entry.name.ends_with(components.extension());
        });
    }

    // Drop old log files.
    //
    // This will work in unexpected ways when traveling between time zones, playing with system clock, or starting OE
    // when DST moves the clock back 1 hour. Should we care? I don't think so. A better implementation would be to add
    // time to `FileStat`, and look at it instead. Originally we had a time field there, but it was dropped, and I'm
    // not keen on bringing it back.
    std::ranges::sort(entries, std::ranges::greater());
    while (!entries.empty() && entries.size() >= count) {
        fs->remove(components.prefix() / entries.back().name);
        entries.pop_back();
    }

    // Open the file that we'll be using.
    std::string name = fmt::format("{}{}{:%Y_%m_%d_%H_%M_%S}{}",
                                   components.stem(),
                                   components.stem().ends_with('_') ? "" : "_",
                                   std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
                                   components.extension());
    return fs->openForWriting(components.prefix() / name);
}

RotatingLogSink::~RotatingLogSink() = default;
