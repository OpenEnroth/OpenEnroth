#pragma once

#include <string_view>
#include <string>
#include <memory>
#include <map>

#include "Utility/Streams/OutputStream.h"
#include "Utility/Memory/Blob.h"

#include "LodInfo.h"

class LodWriter {
 public:
    LodWriter();
    LodWriter(std::string_view path, LodInfo info);
    LodWriter(OutputStream *stream, std::string_view path, LodInfo info);
    ~LodWriter();

    void open(std::string_view path, LodInfo info);
    void open(OutputStream *stream, std::string_view path, LodInfo info);

    void close();

    [[nodiscard]] bool isOpen() const {
        return _stream != nullptr;
    }

    void write(const std::string &filename, const Blob &data);
    void write(const std::string &filename, Blob &&data);

 private:
    std::unique_ptr<OutputStream> _ownedStream;
    OutputStream *_stream = nullptr;
    std::string _path;
    LodInfo _info;
    std::map<std::string, Blob> _files; // Having this one sorted makes implementation simpler.
};
