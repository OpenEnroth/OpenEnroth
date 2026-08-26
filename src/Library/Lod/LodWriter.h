#pragma once

#include <string_view>
#include <string>
#include <memory>
#include <map>

#include "Utility/Streams/OutputStream.h"
#include "Utility/Memory/Blob.h"
#include "Utility/System/Path.h"

#include "LodInfo.h"

class LodWriter {
 public:
    LodWriter();
    LodWriter(const Path &path, LodInfo info);
    LodWriter(OutputStream *stream, LodInfo info);
    ~LodWriter();

    void open(const Path &path, LodInfo info);
    void open(OutputStream *stream, LodInfo info);

    void close();

    [[nodiscard]] bool isOpen() const {
        return _stream != nullptr;
    }

    void write(std::string_view filename, const Blob &data);
    void write(std::string_view filename, Blob &&data);

 private:
    std::unique_ptr<OutputStream> _ownedStream;
    OutputStream *_stream = nullptr;
    LodInfo _info;
    std::map<std::string, Blob> _files; // Having this one sorted makes implementation simpler.
};
