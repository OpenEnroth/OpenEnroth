#include "VidReader.h"

#include <cassert>
#include <algorithm>
#include <utility>
#include <ranges>
#include <unordered_map>
#include <string>
#include <vector>

#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/String/Ascii.h"
#include "Utility/Exception.h"

#include "VidSnapshots.h"

VidReader::VidReader() = default;

VidReader::VidReader(std::string_view path) {
    open(path);
}

VidReader::VidReader(Blob blob) {
    open(std::move(blob));
}

VidReader::~VidReader() = default;

void VidReader::open(std::string_view path) {
    close();
    open(Blob::fromFile(path));
}

void VidReader::open(Blob blob) {
    BlobInputStream stream(blob);

    std::vector<VidEntry> entries;
    deserialize(stream, &entries, tags::via<VidEntry_MM7>);
    std::ranges::sort(entries, std::ranges::less(), &VidEntry::offset);

    std::unordered_map<std::string, VidRegion> files;
    for (size_t i = 0; i < entries.size(); i++) {
        const VidEntry &entry = entries[i];

        std::string name = ascii::toLower(entry.name);
        if (files.contains(name))
            throw Exception("File '{}' is not a valid VID: contains duplicate entries for '{}'", blob.displayPath(), name);

        if (entry.offset > blob.size())
            throw Exception("File '{}' is not a valid VID: entry '{}' points outside the VID file", blob.displayPath(), entry.name);

        size_t nextOffset = (i + 1 == entries.size()) ? blob.size() : entries[i + 1].offset;
        assert(nextOffset >= entry.offset); // Follows from the fact that array is sorted.

        VidRegion region;
        region.offset = entry.offset;
        region.size = nextOffset - entry.offset;
        files.emplace(std::move(name), region);
    }

    // All good, this is a valid VID, can update `this`.
    _vid = std::move(blob);
    _files = std::move(files);
}

void VidReader::close() {
    // Double-closing is OK.
    _vid = Blob();
    _files = {};
}

bool VidReader::exists(std::string_view filename) const {
    assert(isOpen());

    return _files.contains(ascii::toLower(filename));
}

Blob VidReader::read(std::string_view filename) const {
    assert(isOpen());

    const auto pos = _files.find(ascii::toLower(filename));
    if (pos == _files.cend())
        throw Exception("Entry '{}' doesn't exist in VID file '{}'", filename, _vid.displayPath());
    const VidRegion &region = pos->second;

    return _vid.subBlob(region.offset, region.size).withDisplayPath(fmt::format("{}/{}", _vid.displayPath(), filename));
}

std::vector<std::string> VidReader::ls() const {
    assert(isOpen());

    std::vector<std::string> result;
    for (const auto &[name, _] : _files)
        result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}
