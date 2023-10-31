#pragma once

#include <Library/Logger/LogSource.h>

class SdlLogSource: public LogSource {
 public:
    virtual LogLevel level() const override;
    virtual void setLevel(LogLevel level) override;
};
