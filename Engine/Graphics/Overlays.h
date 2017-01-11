#pragma once


/*  282 */
#pragma pack(push, 1)
struct OtherOverlay
{
  OtherOverlay();
  void Reset();

  __int16 field_0;
  __int16 field_2;
  __int16 field_4;
  __int16 field_6;
  __int16 field_8;
  __int16 field_A;
  __int16 field_C;
  __int16 field_E;
  int field_10;
};
#pragma pack(pop)

/*   63 */
#pragma pack(push, 1)
struct OtherOverlayList
{
  inline OtherOverlayList():  //----- (0045848D)
    field_3E8(0), bRedraw(false)
  {}
    
  void Reset();
  int _4418B1(int a2, int a3, int a4, int a5);
  int _4418B6(int uOverlayID, __int16 a3, int a4, int a5, __int16 a6);
  void DrawTurnBasedIcon(int a2);

  OtherOverlay pOverlays[50];
  int field_3E8;
  int bRedraw;
};
#pragma pack(pop)




/*   52 */
#pragma pack(push, 1)
struct OverlayDesc
{
  unsigned __int16 uOverlayID;
  unsigned __int16 uOverlayType;
  unsigned __int16 uSpriteFramesetID;
  __int16 field_6;
};
#pragma pack(pop)



#pragma pack(push, 1)
struct OverlayList
{
  inline OverlayList():  //----- (00458474)
    uNumOverlays(0), pOverlays(nullptr)
  {}

  void ToFile();
  void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
  bool FromFileTxt(const char *Args);
  void InitializeSprites();


  unsigned int uNumOverlays;
  struct OverlayDesc *pOverlays;
};
#pragma pack(pop)




extern struct OtherOverlayList *pOtherOverlayList; // idb
extern struct OverlayList *pOverlayList;