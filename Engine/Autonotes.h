#pragma once
#include <array>



/*  351 */
enum AUTONOTE_TYPE : __int32
{
  AUTONOTE_POTION_RECEPIE = 0,
  AUTONOTE_STAT_HINT = 1,
  AUTONOTE_OBELISK = 2,
  AUTONOTE_SEER = 3,
  AUTONOTE_MISC = 4,
  AUTONOTE_TEACHER = 5,
};

/*  350 */
#pragma pack(push, 1)
struct Autonote
{
  const char    *pText;
  AUTONOTE_TYPE  eType;
};
#pragma pack(pop)
extern std::array<Autonote, 196> pAutonoteTxt; // weak
