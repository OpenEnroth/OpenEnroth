#include "SndReader.h"

#include <algorithm>
#include <utility>
#include <unordered_map>
#include <string>
#include <vector>

#include "Library/Compression/Compression.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/BlobInputStream.h"
#include "Utility/String/Ascii.h"
#include "Utility/Exception.h"

#include "SndSnapshots.h"

SndReader::SndReader() = default;

SndReader::SndReader(std::string_view path) {
    open(path);
}

SndReader::SndReader(Blob blob) {
    open(std::move(blob));
}

SndReader::~SndReader() = default;

void SndReader::open(std::string_view path) {
    close();
    open(Blob::fromFile(path));
}

void SndReader::open(Blob blob) {
    BlobInputStream stream(blob);

    std::vector<SndEntry> entries;
    deserialize(stream, &entries, tags::via<SndEntry_MM7>);

    std::unordered_map<std::string, SndEntry> files;
    for (SndEntry &entry : entries) {
        std::string name = ascii::toLower(entry.name);
        if (files.contains(name))
            throw Exception("File '{}' is not a valid SND: contains duplicate entries for '{}'", blob.displayPath(), name);

        if (entry.offset + entry.size > blob.size())
            throw Exception("File '{}' is not a valid SND: entry '{}' points outside the SND file", blob.displayPath(), entry.name);

        files.emplace(std::move(name), std::move(entry));
    }

    // All good, this is a valid SND, can update `this`.
    _snd = std::move(blob);
    _files = std::move(files);
}

void SndReader::close() {
    // Double-closing is OK.
    _snd = Blob();
    _files = {};
}

bool SndReader::exists(std::string_view filename) const {
    assert(isOpen());

    return _files.contains(ascii::toLower(filename));
}

Blob SndReader::read(std::string_view filename) const {
    assert(isOpen());

    const auto pos = _files.find(ascii::toLower(filename));
    if (pos == _files.cend())
        throw Exception("Entry '{}' doesn't exist in SND file '{}'", filename, _snd.displayPath());
    const SndEntry &entry = pos->second;

    Blob result = _snd.subBlob(entry.offset, entry.size);
    if (entry.decompressedSize && entry.decompressedSize != entry.size)
        result = zlib::uncompress(result, entry.decompressedSize);
    return result.withDisplayPath(fmt::format("{}/{}", _snd.displayPath(), filename));
}

std::vector<std::string> SndReader::ls() const {
    assert(isOpen());

    std::vector<std::string> result;
    for (const auto &[name, _] : _files)
        result.push_back(name);
    std::sort(result.begin(), result.end());
    return result;
}
