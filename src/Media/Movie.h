#pragma once

#include <string>
#include <memory>

#include "Utility/Memory/Blob.h"

class IMovie {
 public:
    IMovie() {}
    virtual ~IMovie() {}

    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;

    virtual bool Play(bool loop = false) = 0;
    virtual bool Stop() = 0;
    virtual bool IsPlaying() const = 0;
    virtual Blob GetFrame() = 0;
    virtual std::string GetFormat() = 0;
    virtual void PlayBink() = 0;

    virtual bool prepare() = 0;
    virtual bool renderFrame() = 0;
};
typedef std::shared_ptr<IMovie> PMovie;
