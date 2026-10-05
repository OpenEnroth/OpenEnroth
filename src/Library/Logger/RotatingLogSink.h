#pragma once

#include <memory>
#include <string_view>

#include "StreamLogSink.h"

class FileSystem;
class Path;
class OutputStream;

class RotatingLogSink : public StreamLogSink {
 public:
    explicit RotatingLogSink(std::string_view path, FileSystem *fs, int count = 16);
    virtual ~RotatingLogSink();

 private:
    static std::unique_ptr<OutputStream> openRotatingStream(const Path &path, FileSystem *fs, int count);
};
