#pragma once




/*  324 */
enum OBJECT_DESC_FLAGS
{
  OBJECT_DESC_NO_SPRITE = 0x1,
  OBJECT_DESC_NO_COLLISION = 0x2,
  OBJECT_DESC_TEMPORARY = 0x4,
  OBJECT_DESC_SFT_LIFETIME = 0x8,
  OBJECT_DESC_UNPICKABLE = 0x10,
  OBJECT_DESC_NO_GRAVITY = 0x20,
  OBJECT_DESC_INTERACTABLE = 0x40,
  OBJECT_DESC_BOUNCE = 0x80,
  OBJECT_DESC_TRIAL_PARTICLE = 0x100,
  OBJECT_DESC_TRIAL_FIRE = 0x200,
  OBJECT_DESC_TRIAL_LINE = 0x400,
};




/*   56 */
#pragma pack(push, 1)
struct ObjectDesc_mm6
{
  inline bool NoSprite() const {return uFlags & OBJECT_DESC_NO_SPRITE;}

  char field_0[32];
  __int16 uObjectID;
  __int16 uRadius;
  __int16 uHeight;
  __int16 uFlags;
  unsigned __int16 uSpriteID;
  __int16 uLifetime;
  unsigned short uParticleTrailColor;
  __int16 uSpeed;
  char uParticleTrailColorR;
  char uParticleTrailColorG;
  char uParticleTrailColorB;
  char field_35_clr;
};

struct ObjectDesc
{
  inline bool NoSprite() const {return uFlags & OBJECT_DESC_NO_SPRITE;}

  char field_0[32];
  __int16 uObjectID;
  __int16 uRadius;
  __int16 uHeight;
  __int16 uFlags;
  unsigned __int16 uSpriteID;
  __int16 uLifetime;
  unsigned int uParticleTrailColor;
  __int16 uSpeed;
  unsigned char uParticleTrailColorR;
  unsigned char uParticleTrailColorG;
  unsigned char uParticleTrailColorB;
  char field_35_clr;
  char field_36_clr;
  char field_37_clr;
};
#pragma pack(pop)

/*   57 */
#pragma pack(push, 1)
struct ObjectList
{
  inline ObjectList():  //----- (004583D5)
    uNumObjects(0), pObjects(nullptr)
  {}

  void ToFile();
  void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
  bool FromFileTxt(const char *Args);
  void InitializeSprites();
  __int16 ObjectIDByItemID(unsigned __int16 uItemID);


  unsigned int uNumObjects;
  struct ObjectDesc *pObjects;
};
#pragma pack(pop)

extern struct ObjectList *pObjectList;