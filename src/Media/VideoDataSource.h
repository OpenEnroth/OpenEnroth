#pragma once

#include <memory>

#include "Utility/Memory/Blob.h"

class IVideoDataSource {
 public:
    IVideoDataSource() {}
    virtual ~IVideoDataSource() {}

    virtual bool Open() = 0;
    virtual void Close() = 0;

    virtual size_t GetFrameRate() = 0;
    virtual size_t GetWidth() = 0;
    virtual size_t GetHeight() = 0;
    virtual Blob GetNextFrame() = 0;
};
typedef std::shared_ptr<IVideoDataSource> PVideoDataSource;
