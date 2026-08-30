#include "RotatingLogSink.h"

#include <cassert>
#include <ranges>
#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/String/Format.h"

RotatingLogSink::RotatingLogSink(std::string_view path, FileSystem *fs, int count): StreamLogSink(openRotatingStream(Path(path), fs, count)) {}

std::unique_ptr<OutputStream> RotatingLogSink::openRotatingStream(const Path &rawPath, FileSystem *fs, int count) {
    assert(fs);

    // parent(), stem() and extension() are lexical, so a dotted path would have us list one directory and write into
    // another - "logs/../logs/oe.log" has a parent of "logs/..". The path comes from a caller, so normalize first.
    Path path = rawPath.normalized();

    // Find existing log files.
    std::vector<DirectoryEntry> entries;
    if (fs->exists(path.parent())) {
        entries = fs->ls(path.parent());
        std::erase_if(entries, [&](const DirectoryEntry &entry) {
            // We're being lazy here and just checking stem & extension. Can do a regex, but that would be an overkill.
            return !(entry.name.starts_with(path.stem()) && entry.name.ends_with(path.extension()));
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
        fs->remove(path.parent() / entries.back().name);
        entries.pop_back();
    }

    // Open the file that we'll be using.
    std::string name = fmt::format("{}{}{:%Y_%m_%d_%H_%M_%S}{}",
                                   path.stem(),
                                   path.stem().ends_with('_') ? "" : "_",
                                   std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
                                   path.extension());
    return fs->openForWriting(path.parent() / name);
}

RotatingLogSink::~RotatingLogSink() = default;
