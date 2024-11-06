#pragma once

#include <memory>
#include <string_view>

#include "StreamLogSink.h"

class FileSystem;
class FileSystemPath;
class OutputStream;

class RotatingLogSink : public StreamLogSink {
 public:
    explicit RotatingLogSink(std::string_view path, FileSystem *fs, int count = 16);
    virtual ~RotatingLogSink();

 private:
    static std::unique_ptr<OutputStream> openRotatingStream(const FileSystemPath &path, FileSystem *fs, int count);
};
