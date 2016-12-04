#pragma once




/*   61 */
#pragma pack(push, 1)
struct Timer
{
  static Timer *Create() {return new Timer;}

  Timer():
    bReady(false), bPaused(false)
  {
    bTackGameTime = 0;
    uStartTime = 0;
    uStopTime = 0;
    uGameTimeStart = 0;
    field_18 = 0;
    uTimeElapsed = 0;
    dt_in_some_format = 0;
    uTotalGameTimeElapsed = 0;
  }

  void Initialize();
  unsigned __int64 Time();
  void Update();
  void Pause();
  void Resume();
  void TrackGameTime();
  void StopGameTime();

  unsigned int bReady;
  unsigned int bPaused;
  int bTackGameTime;
  unsigned int uStartTime;
  unsigned int uStopTime;
  int uGameTimeStart;
  int field_18;
  unsigned int uTimeElapsed;
  int dt_in_some_format;
  unsigned int uTotalGameTimeElapsed;

  static const unsigned int TimeQuant = 128;
  static const unsigned int Minute = 2 * TimeQuant;
  static const unsigned int Hour = 60 * Minute;
  static const unsigned int Day = 24 * Hour;
  static const unsigned int Week = 7 * Day;
  static const unsigned int Month = 4 * Week;
  static const unsigned int Year = 12 * Month;
};

#pragma pack(pop)



extern Timer *pMiscTimer;
extern Timer *pEventTimer;