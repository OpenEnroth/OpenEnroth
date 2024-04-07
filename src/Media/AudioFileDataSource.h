#pragma once

#include <string>
#include <memory>

#include "AudioBaseDataSource.h"

class AudioFileDataSource : public AudioBaseDataSource {
 public:
    explicit AudioFileDataSource(std::string_view file_name);
    virtual ~AudioFileDataSource() {}

    virtual bool Open() override;

 protected:
    std::string sFileName;
};

PAudioDataSource CreateAudioFileDataSource(std::string_view file_name);
