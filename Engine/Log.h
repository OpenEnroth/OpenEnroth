#pragma once

class Log {
 public:
    inline Log() : initialized(false) {}

    bool Initialize();
    void Info(const char *pFormat, ...);
    void Warning(const char *pFormat, ...);

 protected:
    bool initialized;
};
