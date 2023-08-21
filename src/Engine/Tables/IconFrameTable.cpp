#include "Engine/Tables/IconFrameTable.h"

#include <cstring>

#include "Engine/AssetsManager.h"
#include "Utility/String.h"

GraphicsImage *Icon::GetTexture() {
    if (!this->img) {
        this->img = assets->getImage_ColorKey(this->pTextureName);
    }

    return this->img;
}

Icon *IconFrameTable::GetIcon(unsigned int idx) {
    if (idx < pIcons.size()) return &this->pIcons[idx];
    return nullptr;
}

Icon *IconFrameTable::GetIcon(const char *pIconName) {
    for (unsigned int i = 0; i < pIcons.size(); i++) {
        if (iequals(pIconName, this->pIcons[i].GetAnimationName()))
            return &this->pIcons[i];
    }
    return nullptr;
}

//----- (00494F3A) --------------------------------------------------------
unsigned int IconFrameTable::FindIcon(const std::string &pIconName) {
    for (size_t i = 0; i < pIcons.size(); i++) {
        if (iequals(pIconName, this->pIcons[i].GetAnimationName()))
            return i;
    }
    return 0;
}

//----- (00494F70) --------------------------------------------------------
Icon *IconFrameTable::GetFrame(unsigned int uIconID, unsigned int frame_time) {
    //    int v6; // edx@3
    int i;

    if (this->pIcons[uIconID].uFlags & 1 &&
        this->pIcons[uIconID].GetAnimLength() != 0) {
        int t = frame_time;

        t = (t /*/ 8*/) %
            (uint16_t)this->pIcons[uIconID].GetAnimLength();
        t /= 8;
        for (i = uIconID; t > this->pIcons[i].GetAnimTime(); i++)
            t -= this->pIcons[i].GetAnimTime();
        return &this->pIcons[i];
    } else {
        return &this->pIcons[uIconID];
    }
}

//----- (00494FBF) --------------------------------------------------------
void IconFrameTable::InitializeAnimation(unsigned int uIconID) {
    if (uIconID && uIconID <= pIcons.size()) {
        for (int i = uIconID;; ++i) {
            if (!(this->pIcons[i].uFlags & 1)) {
                break;
            }
        }
    }
}

/*
//----- (0049509D) --------------------------------------------------------
int IconFrameTable::FromFileTxt(const char *Args)
{
  //IconFrameTable *v2; // ebx@1
  FILE *v3; // eax@1
  int v4; // esi@3
  void *v5; // eax@10
  FILE *v6; // ST0C_4@12
  char *i; // eax@12
  const char *v8; // ST00_4@15
  int v9; // eax@16
  int v10; // edx@20
  int v11; // ecx@21
  int v12; // eax@22
  signed int j; // edx@25
  IconFrame_MM7 *v14; // ecx@26
  int v15; // esi@26
  int k; // eax@27
  signed int result; // eax@11
  char Buf; // [sp+Ch] [bp-2F8h]@3
  FrameTableTxtLine v19; // [sp+200h] [bp-104h]@4
  FrameTableTxtLine v20; // [sp+27Ch] [bp-88h]@4
  int v21; // [sp+2F8h] [bp-Ch]@3
  int v22; // [sp+2FCh] [bp-8h]@3
  FILE *File; // [sp+300h] [bp-4h]@1
  int Argsa; // [sp+30Ch] [bp+8h]@26

  //v2 = this;
  //TileTable::dtor((TileTable *)this);
  __debugbreak();//Ritor1: this function not used
  v3 = fopen(Args, "r");
  File = v3;
  if ( !v3 )
    Error("IconFrameTable::load - Unable to open file: %s.", Args);
  v4 = 0;
  v21 = 0;
  v22 = 1;
  if ( fgets(&Buf, 490, v3) )
  {
    do
    {
      *strchr(&Buf, 10) = 0;
      memcpy(&v20, frame_table_txt_parser(&Buf, &v19), sizeof(v20));
      if ( v20.uPropCount && *v20.pProperties[0] != 47 )
      {
        if ( v20.uPropCount < 3 )
          Error("IconFrameTable::loadText, too few arguments, %s line %i.",
Args, v22);
        ++v21;
      }
      ++v22;
    }
    while ( fgets(&Buf, 490, File) );
    v4 = v21;
  }
  this->uNumIcons = v4;
  v5 = malloc(32 * v4);//, "I Frames");
  this->pIcons = (IconFrame_MM7 *)v5;
  if ( v5 )
  {
    v6 = File;
    this->uNumIcons = 0;
    fseek(v6, 0, 0);
    for ( i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File) )
    {
      *strchr(&Buf, 10) = 0;
      memcpy(&v20, frame_table_txt_parser(&Buf, &v19), sizeof(v20));
      if ( v20.uPropCount && *v20.pProperties[0] != 47 )
      {
        strcpy(this->pIcons[this->uNumIcons].pAnimationName,
v20.pProperties[0]); strcpy(this->pIcons[this->uNumIcons].pTextureName,
v20.pProperties[1]); v8 = v20.pProperties[2];
        this->pIcons[this->uNumIcons].uFlags = 0;
        if ( iequals(v8, "new") )
        {
          v9 = (int)&this->pIcons[this->uNumIcons].uFlags;
          *(char *)v9 |= 4u;
        }
        this->pIcons[this->uNumIcons].uAnimTime = atoi(v20.pProperties[3]);
        this->pIcons[this->uNumIcons].uAnimLength = 0;
        this->pIcons[this->uNumIcons++].uTextureID = 0;
      }
    }
    fclose(File);
    v10 = 0;
    if ( (signed int)(this->uNumIcons - 1) > 0 )
    {
      v11 = 0;
      do
      {
        v12 = (int)&this->pIcons[v11];
        if ( !(*(char *)(v12 + 60) & 4) )
          *(char *)(v12 + 28) |= 1u;
        ++v10;
        ++v11;
      }
      while ( v10 < (signed int)(this->uNumIcons - 1) );
    }
    for ( j = 0; j < (signed int)this->uNumIcons; *(short *)(Argsa + 26) = v15 )
    {
      v14 = this->pIcons;
      Argsa = (int)&v14[j];
      v15 = *(short *)(Argsa + 24);
      if ( *(char *)(Argsa + 28) & 1 )
      {
        ++j;
        for ( k = (int)&v14[j]; *(char *)(k + 28) & 1; k += 32 )
        {
          v15 += *(short *)(k + 24);
          ++j;
        }
        LOWORD(v15) = v14[j].uAnimTime + v15;
      }
      ++j;
    }
    result = 1;
  }
  else
  {
    fclose(File);
    result = 0;
  }
  return result;
}*/

/*
//----- (0042EB78) --------------------------------------------------------
int IconFrameTable::GetIconAnimLength(unsigned int uIconID)
{
  return 8 * this->pIcons[uIconID].uAnimLength;
}*/
