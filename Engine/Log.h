#pragma once

class Log {
 public:
  inline Log() : initialized(false) {}

  bool Initialize();
  void Info(const wchar_t *pFormat, ...);
  void Warning(const wchar_t *pFormat, ...);

 protected:
  bool initialized;
};

extern Log *logger;
