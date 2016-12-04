#pragma once
#include <array>

typedef unsigned int uint;

#define PID(type, id) (unsigned int)((((8 * (id))) | (type)) & 0xFFFF)  // packed id
#define PID_TYPE(pid) (unsigned int)((pid) & 7)                         // extract type
#define PID_ID(pid)   (unsigned int)(((pid) & 0xFFFF) >> 3)             // extract value


typedef unsigned __int16 _WORD;
typedef unsigned __int64 _QWORD;







/*  297 */
enum SoundType
{
  SOUND_EndTurnBasedMode = 0xCE,
  SOUND_StartTurnBasedMode = 0xCF,
  SOUND_FlipOnExit = 0x4E21,
};



/*  362 */
#pragma pack(push, 1)
struct TravelInfo
{
  char uMapID;
  char pSchedule[7];
  char uDaysCount;
  char field_9[3];
  int x;
  int y;
  int z;
  int direction;
  int field_1C;
};
#pragma pack(pop)












/*  374 */
#pragma pack(push, 1)
struct stat_coord
{
  __int16 x;
  __int16 y;
  __int16 width;
  __int16 height;
};
#pragma pack(pop)
extern std::array<stat_coord, 26> stat_string_coord;

/*  376 */
#pragma pack(push, 1)
struct stru336
{
  int field_0;
  int field_4;
  int field_8;
  int field_C;
  int field_10;
  int field_14;
  __int16 field_18[480];
  __int16 field_3D8[480];
};
#pragma pack(pop)

