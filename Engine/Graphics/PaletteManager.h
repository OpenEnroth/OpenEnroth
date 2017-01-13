#pragma once


/*  148 */
#pragma pack(push, 1)
struct PaletteManager
{
  PaletteManager();
  
  void SetMistColor(unsigned char r, unsigned char g, unsigned char b);
  int ResetNonTestLocked();
  void CalcPalettes_LUT(int a2);
  int ResetNonLocked();
  int LoadPalette(unsigned int uPaletteID);
  int MakeBasePaletteLut(int a2, char *entries);
  void RecalculateAll();
  int LockAll();
  int LockTestAll();
  void SetColorChannelInfo(int uNumRBits, int uNumGBits, int uNumBBits);

  static unsigned __int16 *Get(int a1);
  static unsigned __int16 *Get_Mist_or_Red_LUT(int paletteIdx, int a2, char a3);
  static unsigned __int16 *Get_Dark_or_Red_LUT(int paletteIdx, int a2, char a3);
  static unsigned __int16 *_47C30E_get_palette(int a1, char a2);
  static unsigned __int16 *_47C33F_get_palette(int a1, char a2);

  unsigned __int8 pBaseColors[50][256][3];
  unsigned __int16 pPalette1[50][32][256];
  unsigned __int16 field_D1600[50][32][256];
  unsigned __int16 field_199600_palettes[50][32][256];
  unsigned __int16 field_261600[50][256];
  int pPaletteIDs[50];
  int _num_locked;
  int _pal_lock_test;
  unsigned __int8 pPalette_mistColor[3];
  unsigned char pPalette_tintColor[3];
  char field_267AD6;
  char field_267AD7;
  unsigned int uNumTargetRBits;
  unsigned int uNumTargetGBits;
  unsigned int uNumTargetBBits;
  unsigned int uTargetRMask;
  unsigned int uTargetGMask;
  unsigned int uTargetBMask;
};
#pragma pack(pop)



bool HSV2RGB(float *a1, float *a2, float *a3, float a4, float a5, float a6);
void RGB2HSV(float *a1, float *a2, float a3, float a4, float a5, float *a6);
signed int ReplaceHSV(unsigned int uColor, float a2, float gamma, float a4);
extern PaletteManager *pPaletteManager;