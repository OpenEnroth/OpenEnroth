#include "SndReader.h"

#include <algorithm>
#include <utility>

#include "Library/Compression/Compression.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/String.h"
#include "Utility/Exception.h"

#include "SndSnapshots.h"

SndReader::SndReader() = default;

SndReader::SndReader(std::string_view path) {
    open(path);
}

SndReader::~SndReader() = default;

void SndReader::open(std::string_view path) {
    close();

    Blob blob = Blob::fromFile(path);
    BlobInputStream stream(blob);

    std::vector<SndEntry> entries;
    deserialize(stream, &entries, tags::via<SndEntry_MM7>);

    std::unordered_map<std::string, SndEntry> files;
    for (SndEntry &entry : entries) {
        std::string name = toLower(entry.name);
        if (files.contains(name))
            throw Exception("File '{}' is not a valid SND: contains duplicate entries for '{}'", path, name);

        if (entry.offset + entry.size > blob.size())
            throw Exception("File '{}' is not a valid SND: entry '{}' points outside the SND file", path, entry.name);

        files.emplace(std::move(name), std::move(entry));
    }

    // All good, this is a valid SND, can update `this`.
    _snd = std::move(blob);
    _path = path;
    _files = std::move(files);
}

void SndReader::close() {
    // Double-closing is OK.
    _snd = Blob();
    _path = {};
    _files = {};
}

bool SndReader::exists(const std::string &filename) const {
    assert(isOpen());

    return _files.contains(toLower(filename));
}

Blob SndReader::read(const std::string &filename) const {
    assert(isOpen());

    const auto pos = _files.find(toLower(filename));
    if (pos == _files.cend())
        throw Exception("Entry '{}' doesn't exist in SND file '{}'", filename, _path);
    const SndEntry &entry = pos->second;

    Blob result = _snd.subBlob(entry.offset, entry.size);
    if (entry.decompressedSize)
        result = zlib::uncompress(result, entry.decompressedSize);
    return result;
}

std::vector<std::string> SndReader::ls() const {
    assert(isOpen());

    std::vector<std::string> result;
    for (const auto &[name, _] : _files)
        result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}
