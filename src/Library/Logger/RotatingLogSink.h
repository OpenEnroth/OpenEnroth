#pragma once

#include <memory>

#include "Utility/System/PathView.h"

#include "StreamLogSink.h"

class FileSystem;
class OutputStream;

class RotatingLogSink : public StreamLogSink {
 public:
    explicit RotatingLogSink(PathView path, FileSystem *fs, int count = 16);
    virtual ~RotatingLogSink();

 private:
    static std::unique_ptr<OutputStream> openRotatingStream(const Path &path, FileSystem *fs, int count);
};
