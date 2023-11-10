#pragma once

#include <string>
#include <memory>

#include "AudioBaseDataSource.h"

class AudioFileDataSource : public AudioBaseDataSource {
 public:
    explicit AudioFileDataSource(const std::string &file_name);
    virtual ~AudioFileDataSource() {}

    virtual bool Open() override;

 protected:
    std::string sFileName;
};

PAudioDataSource CreateAudioFileDataSource(const std::string &file_name);
