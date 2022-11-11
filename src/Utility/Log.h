#pragma once

#include <cstdio>

class Log {
 public:
    Log(FILE *infoFile, FILE *warningFile);

    void Info(const char *pFormat, ...);
    void Warning(const char *pFormat, ...);

 private:
    FILE *infoFile_ = nullptr;
    FILE *warningFile_ = nullptr;
};
