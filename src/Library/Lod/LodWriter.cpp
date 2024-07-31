#include "LodWriter.h"

#include <utility>
#include <vector>
#include <memory>

#include "Library/Serialization/Serialization.h"
#include "Library/Snapshots/SnapshotSerialization.h"

#include "Utility/Streams/FileOutputStream.h"
#include "Utility/String/Ascii.h"

#include "LodSnapshots.h"

LodWriter::LodWriter() {}

LodWriter::LodWriter(std::string_view path, LodInfo info) {
    open(path, std::move(info));
}

LodWriter::LodWriter(OutputStream *stream, LodInfo info) {
    open(stream, std::move(info));
}

LodWriter::~LodWriter() {
    close();
}

void LodWriter::open(std::string_view path, LodInfo info) {
    std::unique_ptr<OutputStream> ownedStream = std::make_unique<FileOutputStream>(path); // If this throws, no field is overwritten.
    open(ownedStream.get(), std::move(info));
    _ownedStream = std::move(ownedStream);
}

void LodWriter::open(OutputStream *stream, LodInfo info) {
    assert(stream);

    close();

    _stream = stream;
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
    _files.clear(); // Important to release the Blobs first, as they might point into a file that we're about to overwrite...
    _ownedStream = {}; // ...here.
    _stream = {};
    _info = {};
}

void LodWriter::write(std::string_view filename, const Blob &data) {
    write(filename, Blob::share(data));
}

void LodWriter::write(std::string_view filename, Blob &&data) {
    assert(isOpen());

    _files.insert_or_assign(ascii::toLower(filename), std::move(data));
}
