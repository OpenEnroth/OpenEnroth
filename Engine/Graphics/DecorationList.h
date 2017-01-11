#pragma once




/*  321 */
enum DECORATION_DESC_FLAGS
{
  DECORATION_DESC_MOVE_THROUGH = 0x0001,
  DECORATION_DESC_DONT_DRAW = 0x0002,
  DECORATION_DESC_FLICKER_SLOW = 0x0004,
  DECORATION_DESC_FICKER_AVERAGE = 0x0008,
  DECORATION_DESC_FICKER_FAST = 0x0010,
  DECORATION_DESC_MARKER = 0x0020,
  DECORATION_DESC_SLOW_LOOP = 0x0040,
  DECORATION_DESC_EMITS_FIRE = 0x0080,
  DECORATION_DESC_SOUND_ON_DAWN = 0x0100,
  DECORATION_DESC_SOUND_ON_DUSK = 0x0200,
  DECORATION_DESC_EMITS_SMOKE = 0x0400,
};



/*   54 */
#pragma pack(push, 1)
struct DecorationDesc_mm6
{
  inline bool CanMoveThrough() {return (uFlags & DECORATION_DESC_MOVE_THROUGH) != 0;}
  inline bool DontDraw()       {return (uFlags & DECORATION_DESC_DONT_DRAW) != 0;} 
  inline bool SoundOnDawn()    {return (uFlags & DECORATION_DESC_SOUND_ON_DAWN) != 0;}
  inline bool SoundOnDusk()    {return (uFlags & DECORATION_DESC_SOUND_ON_DUSK) != 0;}

  char pName[32];
  char field_20[32];
  __int16 uType;
  unsigned __int16 uDecorationHeight;
  __int16 uRadius;
  __int16 uLightRadius;
  unsigned __int16 uSpriteID;
  __int16 uFlags;
  __int16 uSoundID;
  __int16 _pad;
  //unsigned __int8 uColoredLightRed;
  //unsigned __int8 uColoredLightGreen;
  //unsigned __int8 uColoredLightBlue;
  //char __padding;
};

struct DecorationDesc: public DecorationDesc_mm6
{
  unsigned __int8 uColoredLightRed;
  unsigned __int8 uColoredLightGreen;
  unsigned __int8 uColoredLightBlue;
  char __padding;
};
#pragma pack(pop)

/*   55 */
#pragma pack(push, 1)
struct DecorationList
{
  inline DecorationList():   //----- (00458380)
    pDecorations(nullptr), uNumDecorations(0)
  {}

  void ToFile();
  void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
  bool FromFileTxt(const char *Args);
  void InitializeDecorationSprite(unsigned int uDecID);
  unsigned __int16 GetDecorIdByName(const char *pName);

  unsigned int uNumDecorations;
  struct DecorationDesc *pDecorations;
};
#pragma pack(pop)


extern struct DecorationList *pDecorationList;

void RespawnGlobalDecorations();