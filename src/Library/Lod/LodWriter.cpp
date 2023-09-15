#include "LodWriter.h"

#include <utility>
#include <vector>

#include "Library/Serialization/Serialization.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String.h"

#include "LodSnapshots.h"

LodWriter::LodWriter() {}

LodWriter::LodWriter(std::string_view path, LodInfo info) {
    open(path, std::move(info));
}

LodWriter::LodWriter(OutputStream *stream, std::string_view path, LodInfo info) {
    open(stream, path, std::move(info));
}

LodWriter::~LodWriter() = default;

void LodWriter::open(std::string_view path, LodInfo info) {
    _ownedStream = std::make_unique<FileOutputStream>(path); // If this throws, no field is overwritten.
    open(_ownedStream.get(), path, std::move(info));
}

void LodWriter::open(OutputStream *stream, std::string_view path, LodInfo info) {
    assert(stream);

    _stream = stream;
    _path = path;
    _info = std::move(info);
}

void LodWriter::close() {
    if (!isOpen())
        return; // Double-closing is OK.

    // Write out LOD header.
    LodHeader header;
    header.signature = "LOD";
    header.version = toString(_info.version);
    header.description = _info.description;
    header.numDirectories = 1;
    serialize(header, _stream, tags::via<LodHeader_MM6>);

    // Write out root entry.
    size_t dataSize = 0;
    for (const auto &[_, data] : _files)
        dataSize += data.size();
    size_t indexSize = _files.size() * fileEntrySize(_info.version);

    LodEntry directoryEntry;
    directoryEntry.name = _info.rootName;
    directoryEntry.dataOffset = sizeof(LodHeader_MM6) + sizeof(LodEntry_MM6);
    directoryEntry.dataSize = indexSize + dataSize;
    directoryEntry.numItems = _files.size();
    serialize(directoryEntry, _stream, tags::via<LodEntry_MM6>);

    // Write out file entries.
    size_t currentOffset = indexSize;
    std::vector<LodEntry> fileEntries;
    for (const auto &[name, data] : _files) {
        LodEntry &entry = fileEntries.emplace_back();
        entry.name = name;
        entry.dataOffset = currentOffset;
        entry.dataSize = data.size();
        entry.numItems = 0;

        currentOffset += data.size();
    }

    if (_info.version == LOD_VERSION_MM8) {
        serialize(fileEntries, _stream, tags::unsized, tags::via<LodFileEntry_MM8>);
    } else {
        serialize(fileEntries, _stream, tags::unsized, tags::via<LodEntry_MM6>);
    }

    for (const auto &[_, data] : _files)
        _stream->write(data.string_view());

    // Close shop.
    _ownedStream = {};
    _path = {};
    _info = {};
    _files.clear();
}

void LodWriter::write(const std::string &filename, const Blob &data) {
    write(filename, Blob::share(data));
}

void LodWriter::write(const std::string &filename, Blob &&data) {
    assert(isOpen());

    _files.emplace(toLower(filename), std::move(data));
}
