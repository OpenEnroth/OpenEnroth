#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>

#include "Engine/Engine.h"

#include "Texture.h"
#include "../Tables/FrameTableInc.h"
#include "../LOD.h"
#include "PaletteManager.h"
#include "../ZlibWrapper.h"


#include "../OurMath.h"








struct TextureFrameTable *pTextureFrameTable;



stru355 stru_4E82A4 = {0x20, 0x41, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, 0xFF000000};
stru355 stru_4EFCBC = {0x20, 0x41, 0, 0x10, 0x7C00, 0x3E0, 0x1F, 0x8000};

//Texture_MM7 pTex_F7CE30;








struct ImageLoader
{
    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format) = 0;
};



/*  245 */
#pragma pack(push, 1)
struct PCXHeader1
{
  char magic;
  char version;
  char encoding;
  char bpp;
  __int16 left;
  __int16 up;
  __int16 right;
  __int16 bottom;
  __int16 hres;
  __int16 vres;
};
#pragma pack(pop)

/*  246 */
#pragma pack(push, 1)
struct PCXHeader2
{
  char reserved;
  char planes;
  __int16 pitch;
  __int16 palette_info;
};
#pragma pack(pop)









//----- (0044E054) --------------------------------------------------------
void TextureFrameTable::ToFile()
{
  TextureFrameTable *v1; // esi@1
  FILE *v2; // eax@1
  FILE *v3; // edi@1

  v1 = this;
  v2 = fopen("data\\dtft.bin", "wb");
  v3 = v2;
  if ( !v2 )
    Error("Unable to save dtft.bin!");
  fwrite(v1, 4u, 1u, v2);
  fwrite(v1->pTextures, 0x14u, v1->sNumTextures, v3);
  fclose(v3);
}

//----- (0044E0A0) --------------------------------------------------------
void TextureFrameTable::FromFile(void *data_mm6, void *data_mm7, void *data_mm8)
{
  uint num_mm6_frames = data_mm6 ? *(int *)data_mm6 : 0,
       num_mm7_frames = data_mm7 ? *(int *)data_mm7 : 0,
       num_mm8_frames = data_mm8 ? *(int *)data_mm8 : 0;

  sNumTextures = num_mm6_frames + num_mm7_frames + num_mm8_frames;
  Assert(sNumTextures);
  Assert(!num_mm8_frames);

  pTextures = (TextureFrame *)malloc(sNumTextures * sizeof(TextureFrame));

  memcpy(pTextures,                                   (char *)data_mm7 + 4, num_mm7_frames * sizeof(TextureFrame));
  memcpy(pTextures + num_mm7_frames,                  (char *)data_mm6 + 4, num_mm6_frames * sizeof(TextureFrame));
  memcpy(pTextures + num_mm6_frames + num_mm7_frames, (char *)data_mm8 + 4, num_mm8_frames * sizeof(TextureFrame));
}

//----- (0044E0ED) --------------------------------------------------------
void TextureFrameTable::LoadAnimationSequenceAndPalettes( signed int uIconID )
{
  //TextureFrameTable *v3; // ebx@1
  unsigned int i; // edi@3

  //v3 = this;
  if ( (uIconID <= this->sNumTextures) && uIconID >= 0 )
  {
    for ( i = uIconID; ; ++i )
    {
      this->pTextures[i].uTextureID = pBitmaps_LOD->LoadTexture(this->pTextures[i].pTextureName, TEXTURE_DEFAULT);

      if (this->pTextures[i].uTextureID != -1)
        pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id2 = pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[this->pTextures[i].uTextureID].palette_id1);
      //result = (unsigned int)v3->pTextures;
      //if ( !(*(char *)(result + i * 20 + 18) & 1) )
      if( this->pTextures[i].uFlags & 1)
        break;
    }
  }
  return;
}

//----- (0044E163) --------------------------------------------------------
signed int TextureFrameTable::FindTextureByName(const char *Str2)
{
  if ( (signed int)this->sNumTextures <= 0 )
    return -1;
  for ( int i = 0; (signed int)i < (signed int)this->sNumTextures; ++i )
  {
    if ( !_stricmp(this->pTextures[i].pTextureName, Str2) )
      return i;
  }
  return -1;
}

//----- (0044E19A) --------------------------------------------------------
int TextureFrameTable::GetFrameTexture(int uFrameID, signed int a3)
{
  int v3; // esi@1
  TextureFrame *v4; // edi@1
  TextureFrame *v5; // ecx@1
  __int16 v6; // dx@2
  int v7; // edx@3
  char *i; // eax@3
  int v9; // ecx@5
  unsigned int result; // eax@6

  v3 = uFrameID;
  v4 = this->pTextures;
  v5 = &v4[uFrameID];
  if ( v5->uFlags & 1 && (v6 = v5->uAnimLength) != 0 )
  {
    v7 = (a3 >> 3) % v6;
    for ( i = (char *)&v5->uAnimTime; ; i += 20 )
    {
      v9 = *(short *)i;
      if ( v7 <= v9 )
        break;
      v7 -= v9;
      ++v3;
    }
    result = v4[v3].uTextureID;
  }
  else
  {
    result = v5->uTextureID;
  }
  return result;
}






//----- (0040F806) --------------------------------------------------------
void *Texture_MM7::UnzipPalette()
{
    Texture_MM7 *v1; // esi@1

    void *v2; // edi@1
    Texture_MM7 *pSource; // [sp+0h] [bp-4h]@1

    pSource = this;
    v1 = this;
    v2 = malloc(this->uDecompressedSize);
    zlib::MemUnzip(v2, (unsigned int *)&pSource, v1->paletted_pixels, v1->uTextureSize);
    return v2;
}

//----- (0040F77C) --------------------------------------------------------
void Texture_MM7::Release()
{
  if (this)
  {
    pName[0] = 0;

    if (pBits & 0x0400)
    {
		__debugbreak();
	}
	
	  free(paletted_pixels);
      free(pPalette16);
      free(pPalette24);
    

      paletted_pixels = nullptr;
    pLevelOfDetail1 = nullptr;
    //pLevelOfDetail2 = nullptr;
    //pLevelOfDetail3 = nullptr;

    if (d3d11_srv)
    {
      if (d3d11_desc)
      {
        delete d3d11_desc;
        d3d11_desc = nullptr;
      }

      extern void d3d11_release(struct ID3D11ShaderResourceView *);
      d3d11_release(d3d11_srv);
      d3d11_srv = nullptr;
    }

    pPalette16 = nullptr;
    pPalette24 = nullptr;

    uSizeOfMaxLevelOfDetail = 0;
    uTextureSize = 0;
    uTextureHeight = 0;
    uTextureWidth = 0;
    uHeightLn2 = 0;
    uWidthLn2 = 0;
    palette_id1 = 0;
    palette_id2 = 0;
    pBits &= 0xFFFF0000;
  }
}

//----- (0040F5BE) --------------------------------------------------------
Texture_MM7::Texture_MM7()
{
  pName[0] = 0;
  uSizeOfMaxLevelOfDetail = 0;
  uTextureSize = 0;
  uTextureHeight = 0;
  uTextureWidth = 0;
  uHeightLn2 = 0;
  uWidthLn2 = 0;
  palette_id1 = 0;
  palette_id2 = 0;
  paletted_pixels = nullptr;
  //pLevelOfDetail3 = nullptr;
  //pLevelOfDetail2 = nullptr;
  pLevelOfDetail1 = nullptr;
  pPalette16 = nullptr;
  pPalette24 = nullptr;

  d3d11_srv = nullptr;
  d3d11_desc = nullptr;
}


//----- (0044E1EC) --------------------------------------------------------
int TextureFrameTable::FromFileTxt(const char *Args)
{
  TextureFrameTable *v2; // ebx@1
  FILE *v3; // eax@1
  int v4; // esi@3
  const void *v5; // ST0C_4@10
  void *v6; // eax@10
  FILE *v7; // ST0C_4@12
  char *i; // eax@12
  signed int v9; // esi@15
  int v10; // eax@17
  int v11; // edx@22
  int v12; // ecx@23
  int v13; // eax@24
  signed int j; // eax@27
  TextureFrame *v15; // edx@28
  int v16; // esi@28
  int k; // ecx@29
  char Buf; // [sp+Ch] [bp-2F8h]@3
  FrameTableTxtLine v20; // [sp+200h] [bp-104h]@4
  int v21; // [sp+27Ch] [bp-88h]@4
  char *Str1; // [sp+280h] [bp-84h]@5
  char *Str; // [sp+284h] [bp-80h]@15
  int v24; // [sp+2F8h] [bp-Ch]@3
  int v25; // [sp+2FCh] [bp-8h]@3
  FILE *File; // [sp+300h] [bp-4h]@1
  int Argsa; // [sp+30Ch] [bp+8h]@28

  v2 = this;
  v3 = fopen(Args, "r");
  File = v3;
  if ( !v3 )
    Error("CTextureFrameTable::load - Unable to open file: %s.", Args);

  v4 = 0;
  v24 = 0;
  v25 = 1;
  if ( fgets(&Buf, 490, v3) )
  {
    do
    {
      *strchr(&Buf, 10) = 0;
      memcpy(&v21, txt_file_frametable_parser(&Buf, &v20), 0x7Cu);
      __debugbreak(); // warning C4700: uninitialized local variable 'Str1' used
      if ( v21 && *Str1 != 47 )
      {
        if ( v21 < 2 )
          Error("CTextureFrameTable::load, too few arguments, %s line %i.", Args, v25);
        ++v24;
      }
      ++v25;
    }
    while ( fgets(&Buf, 490, File) );
    v4 = v24;
  }
  v5 = v2->pTextures;
  v2->sNumTextures = v4;
  v6 = malloc(20 * v4);
  v2->pTextures = (TextureFrame *)v6;
  if ( !v6 )
    Error("CTextureFrameTable::load - Out of Memory!");
  v7 = File;
  v2->sNumTextures = 0;
  fseek(v7, 0, 0);
  for ( i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File) )
  {
    *strchr(&Buf, 10) = 0;
    memcpy(&v21, txt_file_frametable_parser(&Buf, &v20), 0x7Cu);
    if ( v21 && *Str1 != 47 )
    {
      strcpy(v2->pTextures[v2->sNumTextures].pTextureName, Str1);
      __debugbreak(); // warning C4700: uninitialized local variable 'Str' used
      v2->pTextures[v2->sNumTextures].uAnimTime = atoi(Str);
      v9 = 2;
      for ( v2->pTextures[v2->sNumTextures].uFlags = 0; v9 < v21; ++v9 )
      {
        if ( !_stricmp((&Str1)[4 * v9], "New") )
        {
          //v10 = (int)&v2->pTextures[v2->sNumTextures].uFlags;
          v2->pTextures[v2->sNumTextures].uFlags |= 2;
        }
      }
      ++v2->sNumTextures;
    }
  }
  fclose(File);
  v11 = 0;
  if ( (signed int)(v2->sNumTextures - 1) > 0 )
  {
    v12 = 0;
    do
    {
      v13 = (int)&v2->pTextures[v12];
      if ( !(*(char *)(v13 + 38) & 2) )
        *(char *)(v13 + 18) |= 1u;
      ++v11;
      ++v12;
    }
    while ( v11 < (signed int)(v2->sNumTextures - 1) );
  }
  for ( j = 0; j < (signed int)v2->sNumTextures; *(short *)(Argsa + 16) = v16 )
  {
    v15 = v2->pTextures;
    Argsa = (int)&v15[j];
    v16 = *(short *)(Argsa + 14);
    if ( *(char *)(Argsa + 18) & 1 )
    {
      ++j;
      for ( k = (int)&v15[j]; *(char *)(k + 18) & 1; k += 20 )
      {
        v16 += *(short *)(k + 14);
        ++j;
      }
      LOWORD(v16) = v15[j].uAnimTime + v16;
    }
    ++j;
  }
  return 1;
}

//----- (00451007) --------------------------------------------------------
int stru350::sub_451007_scale_image_bicubic(unsigned short *pSrc, int srcWidth, int srcHeight, int srcPitch,          //changing this to some library function might be a good idea
	unsigned short *pDst, int dstWidth, int dstHeight, int dstPitch,
	int a10, int a11)
{
	int result; // eax@1
	float v17; // ST3C_4@12
	float v18; // ST38_4@12
	unsigned int v19; // esi@12
	int v21; // eax@18
	unsigned int v22; // ecx@25
	unsigned int v23; // eax@29
	unsigned int heightRatioPlusOne; // [sp+Ch] [bp-7Ch]@12
	unsigned int widthRatio; // [sp+Ch] [bp-7Ch]@218
	unsigned int heightRatio; // [sp+14h] [bp-74h]@12
	unsigned int widthRatioPlusOne; // [sp+14h] [bp-74h]@218
	//  int v160; // [sp+3Ch] [bp-4Ch]@13
	unsigned __int16 *v175; // [sp+4Ch] [bp-3Ch]@13
	unsigned __int16 *v193; // [sp+5Ch] [bp-2Ch]@7
	//signed int v231; // [sp+78h] [bp-10h]@7
	__int64 v240; // [sp+7Ch] [bp-Ch]@12
	unsigned int v251; // [sp+80h] [bp-8h]@218
	unsigned int v252; // [sp+84h] [bp-4h]@218
	float a6s; // [sp+A0h] [bp+18h]@218
	float a6t; // [sp+A0h] [bp+18h]@218
	unsigned int a6b; // [sp+A0h] [bp+18h]@218
	int field_0_bits;
	int field_20_bits;

	int field0value = this->field_0.field_C;
	switch (field0value)
	{
	case 8: field_0_bits = 1;
		break;
	case 16: field_0_bits = 2;
		break;
	case 32: field_0_bits = 4;
		break;
	default:
		return field0value;
	}
	int field20value = this->field_20.field_C;
	switch (field20value)
	{
	case 8: field_20_bits = 1;
		break;
	case 16: field_20_bits = 2;
		break;
	case 32: field_20_bits = 4;
		break;
	default:
		return field20value;
	}

	result = (int)pDst;
	v193 = pDst;
	if (dstHeight <= 0)
		return result;

	//do
	for (int height = 0; height < dstHeight; height++)
	{
		for (int width = 0; width < dstWidth; width++)
		{
			a6s = (double)width / (double)dstWidth * (double)srcWidth;
			widthRatio = bankersRounding(a6s);
			a6t = (double)(width + 1) / (double)dstWidth * (double)srcWidth;
			widthRatioPlusOne = bankersRounding(a6t);

			v17 = (double)height / (double)dstHeight * (double)srcHeight;
			heightRatio = bankersRounding(v17);
			v18 = (double)(height + 1) / (double)dstHeight * (double)srcHeight;
			heightRatioPlusOne = bankersRounding(v18);

			v251 = 0;
			v19 = (heightRatioPlusOne - heightRatio) * (widthRatioPlusOne - widthRatio);
			v252 = 0;
			a6b = 0;
			v240 = 0i64;

			v175 = (unsigned short*)((char *)pSrc + field_0_bits * (widthRatio + srcPitch * heightRatio));
			for (int heightDiff = 0; heightDiff < heightRatioPlusOne - heightRatio; heightDiff++)
			{
				//int ratioDiff = widthRatioPlusOne - widthRatio;
				for (int ratioDiff = 0; ratioDiff < widthRatioPlusOne - widthRatio; ratioDiff++)
				{
					if (field0value == 32)
						v21 = _450FB1(((int*)v175)[ratioDiff]);
					else if (field0value == 16)
						v21 = _450FB1(((_WORD*)v175)[ratioDiff]);
					else if (field0value == 8)
						v21 = _450FB1(((unsigned __int8*)v175)[ratioDiff]);
					v240 += ((unsigned int)v21 >> 24);
					a6b += BYTE2(v21);
					v252 += BYTE1(v21);
					v251 += (unsigned __int8)v21;
				}
				if (field0value == 32)
					v175 += 2 * srcPitch;
				else if (field0value == 16)
					v175 += srcPitch;
				else if (field0value == 8)
					v175 = (unsigned short*)((char *)v175 + 2 * srcPitch);
			}

			v22 = (unsigned int)v240 / ((heightRatioPlusOne - heightRatio) * (widthRatioPlusOne - widthRatio));
			if (v19)
			{
				a6b /= v19;
				v252 /= v19;
				v251 /= v19;
			}
			if (v22 != 255)
				v22 &= 0x7FFFFFFFu;
			v23 = _450F55(v251 | ((v252 | ((a6b | (v22 << 8)) << 8)) << 8));
			*(_DWORD *)v193 = v23; //Invalid memory access
			v193 = (unsigned __int16 *)((char *)v193 + field_20_bits);
		}
		v193 = (unsigned __int16 *)((char *)v193 + field_20_bits * (dstPitch - dstWidth));
		//++v231;
		result = height;
	}
	//while(v231 < dstHeight);
	return result;
}


//----- (00450DDE) --------------------------------------------------------
stru350 *stru350::_450DDE()
{
	_450DF1(&stru_4E82A4, &stru_4E82A4);
	return this;
}

//----- (00450DF1) --------------------------------------------------------
bool stru350::_450DF1(const stru355 *p1, const stru355 *p2)
{
	//stru350 *v3; // esi@1
	//void *result; // eax@1
	unsigned int v5; // ecx@2
	int v6; // edi@2
	int v7; // edx@2
	unsigned int v8; // ecx@8
	int v9; // edi@8
	int v10; // edx@8
	int v11; // ecx@12
	int v12; // edi@12
	unsigned int v13; // ecx@12
	int v14; // edx@12
	int v15; // ecx@16
	unsigned int v16; // edx@16
	int v17; // ecx@16
	int v18; // edi@16
	char v19; // zf@20
	unsigned int v20; // ecx@21
	int v21; // edi@21
	int v22; // edx@21
	unsigned int v23; // ecx@27
	int v24; // edi@27
	int v25; // edx@27
	int v26; // ecx@31
	int v27; // edi@31
	unsigned int v28; // ecx@31
	int v29; // edx@31
	int v30; // ebx@35
	int v31; // ecx@35
	int v32; // edi@35
	int v33; // edx@35
	unsigned int i; // ecx@35
	int v35; // ecx@39
	unsigned int v36; // edx@39
	int v37; // ecx@39
	int v38; // ebx@39

	//v3 = this;
	memcpy(&field_0, p1, sizeof(stru355));
	memcpy(&field_20, p2, sizeof(stru355));
	//result = memcpy(&v3->field_20, p2, 0x20u);
	//LOBYTE(result) = 1;
	if (field_0.field_4 & 1)
	{
		v5 = field_0.field_1C;
		v6 = 0;
		v7 = 0;
		while (!(v5 & 1))
		{
			++v6;
			v5 >>= 1;
		}
		do
		{
			v5 >>= 1;
			++v7;
		} while (v5 & 1);
		field_40 = 32 - v7 - v6;
	}
	else
	{
		field_40 = 0;
	}
	v8 = field_0.field_10;
	v9 = 0;
	v10 = 0;
	while (!(v8 & 1))
	{
		++v9;
		v8 >>= 1;
	}
	do
	{
		v8 >>= 1;
		++v10;
	} while (v8 & 1);
	v11 = 24 - v10 - v9;
	v12 = 0;
	field_48 = v11;
	v13 = field_0.field_14;
	v14 = 0;
	while (!(v13 & 1))
	{
		++v12;
		v13 >>= 1;
	}
	do
	{
		v13 >>= 1;
		++v14;
	} while (v13 & 1);
	v15 = 16 - v14;
	v16 = field_0.field_18;
	field_50 = v15 - v12;
	v17 = 0;
	v18 = 0;
	while (!(v16 & 1))
	{
		++v17;
		v16 >>= 1;
	}
	do
	{
		v16 >>= 1;
		++v18;
	} while (v16 & 1);
	v19 = (field_20.field_4 & 1) == 0;
	field_58 = v17 - v18 + 8;
	if (v19)
	{
		field_44 = 0;
	}
	else
	{
		v20 = field_20.field_1C;
		v21 = 0;
		v22 = 0;
		while (!(v20 & 1))
		{
			++v21;
			v20 >>= 1;
		}
		do
		{
			v20 >>= 1;
			++v22;
		} while (v20 & 1);
		field_44 = 32 - v22 - v21;
	}
	v23 = field_20.field_10;
	v24 = 0;
	v25 = 0;
	while (!(v23 & 1))
	{
		++v24;
		v23 >>= 1;
	}
	do
	{
		v23 >>= 1;
		++v25;
	} while (v23 & 1);
	v26 = 24 - v25 - v24;
	v27 = 0;
	field_4C = v26;
	v28 = field_20.field_14;
	v29 = 0;
	while (!(v28 & 1))
	{
		++v27;
		v28 >>= 1;
	}
	do
	{
		v28 >>= 1;
		++v29;
	} while (v28 & 1);
	v30 = 0;
	v31 = 16 - v29 - v27;
	v32 = field_20.field_18;
	field_54 = v31;
	v33 = 0;
	for (i = v32; !(i & 1); i >>= 1)
		++v30;
	do
	{
		i >>= 1;
		++v33;
	} while (i & 1);
	v35 = 32 - v33;
	v36 = v32;
	field_5C = v35 - v30;
	v37 = 0;
	v38 = 0;
	while (!(v36 & 1))
	{
		++v37;
		v36 >>= 1;
	}
	do
	{
		v36 >>= 1;
		++v38;
	} while (v36 & 1);
	field_5C = v37 - v38 + 8;
	return true;
}

//----- (00450F55) --------------------------------------------------------
unsigned int stru350::_450F55(int a2)
{
	int v2 = a2 & stru_4E82A4.field_1C;
	if (field_20.field_4 & 1)
		v2 = (unsigned int)v2 >> this->field_44;
	return v2 & field_20.field_1C |
		field_20.field_10 & ((a2 & (unsigned int)stru_4E82A4.field_10) >> field_4C) |
		field_20.field_14 & ((a2 & (unsigned int)stru_4E82A4.field_14) >> field_54) |
		field_20.field_18 & ((a2 & (unsigned int)stru_4E82A4.field_18) >> field_5C);
}

//----- (00450FB1) --------------------------------------------------------
int stru350::_450FB1(int a2)
{
	int v2 = 0;
	int v4 = field_0.field_4 & 1;
	if (v4)
		v2 = a2 & field_0.field_1C;
	if (v4)
		v2 <<= field_40;
	return v2 | ((a2 & field_0.field_10) << field_48) | ((a2 & field_0.field_14) << field_50) | ((a2 & field_0.field_18) << field_58);
}





typedef bool (*ImageFormatConverter)(unsigned int num_pixels, const void *src, void *dst);


unsigned __int32 R5G6B5_to_A8R8G8B8(unsigned __int16 color16, unsigned char alpha)
{
    unsigned __int32 c = color16;
    unsigned int b = (c & 31) * 8;
    unsigned int g = ((c >> 5) & 63) * 4;
    unsigned int r = ((c >> 11) & 31) * 8;

    return ((unsigned int)alpha << 24) | (r << 16) | (g << 8) | b;
}

unsigned __int16 A8R8G8B8_to_R5G6B5(unsigned __int32 c)
{
    unsigned __int32 b = ((c & 0xFF) / 8) & 31;
    unsigned __int32 g = (((c >> 8) & 0xFF) / 4) & 63;
    unsigned __int32 r = (((c >> 16) & 0xFF) / 8) & 31;

    return (unsigned __int16)(
        (r << 11) | (g << 5) | b
    );
}

bool Image_A8R8G8B8_to_R5G6B5(unsigned int num_pixels, const void *src_pixels, void *dst_pixels)
{
    auto src = (unsigned __int32 *)src_pixels;
    auto dst = (unsigned __int16 *)dst_pixels;

    for (unsigned int i = 0; i < num_pixels; ++i)
        dst[i] = A8R8G8B8_to_R5G6B5(src[i]);

    return true;
}





struct Image16bit_LOD_Loader : public ImageLoader
{
    inline Image16bit_LOD_Loader(LODFile_IconsBitmaps *lod, const wchar_t *name)
    {
        wcscpy(this->name, name);
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);

    wchar_t               name[1024];
    LODFile_IconsBitmaps *lod;
};


bool Image16bit_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    char namea[1024];
    sprintf(namea, "%S", name);

    auto tex = lod->GetTexture(lod->LoadTexture(namea, TEXTURE_16BIT_PALETTE));
    if (tex->pBits & 512)
        Log::Warning(L"Alpha texture is loaded as Image16bit (%s)", name);

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int16[width * height];
        if (pixels)
        {
            for (unsigned int y = 0; y < height; ++y)
                for (unsigned int x = 0; x < width; ++x)
                {
                    auto index = paletted_pixels[y * width + x];
                    auto pixel = tex->pPalette16[index];
                    
                    pixels[y * width + x] = pixel;
                }
            *out_width = width;
            *out_height = height;
            *out_pixels = pixels;
            *out_format = IMAGE_FORMAT_R5G6B5;
        }
    }

    return *out_pixels != nullptr;
}






struct Alpha_LOD_Loader : public ImageLoader
{
    inline Alpha_LOD_Loader(LODFile_IconsBitmaps *lod, const wchar_t *name)
    {
        wcscpy(this->name, name);
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);

    wchar_t               name[1024];
    LODFile_IconsBitmaps *lod;
};


bool Alpha_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    char namea[1024];
    sprintf(namea, "%S", name);

    auto tex = lod->GetTexture(lod->LoadTexture(namea, TEXTURE_16BIT_PALETTE));
    if (~tex->pBits & 512)
        Log::Warning(L"ColorKey texture is loaded as Alpha (%s)", name);

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int32[width * height];
        if (pixels)
        {
            for (unsigned int y = 0; y < height; ++y)
                for (unsigned int x = 0; x < width; ++x)
                {
                    auto index = paletted_pixels[y * width + x];
                    auto pixel = tex->pPalette16[index];
                    if (index == 0)
                        pixels[y * width + x] = 0x00000000;
                    else
                    {
                        extern unsigned __int32 Color32(unsigned __int16 color16);
                        pixels[y * width + x] = R5G6B5_to_A8R8G8B8(pixel, 255);
                    }
                }
            *out_width = width;
            *out_height = height;
            *out_pixels = pixels;
            *out_format = IMAGE_FORMAT_A8R8G8B8;
        }
    }

    return *out_pixels != nullptr;
}








struct ColorKey_LOD_Loader : public ImageLoader
{
    inline ColorKey_LOD_Loader(LODFile_IconsBitmaps *lod, const wchar_t *name, unsigned __int16 colorkey)
    {
        wcscpy(this->name, name);
        this->colorkey = colorkey;
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);

    wchar_t               name[1024];
    unsigned __int16      colorkey;
    LODFile_IconsBitmaps *lod;
};



bool ColorKey_LOD_Loader::Load(unsigned int *out_width, unsigned int *out_height, void **out_pixels, IMAGE_FORMAT *out_format)
{
    *out_width = 0;
    *out_height = 0;
    *out_pixels = nullptr;
    *out_format = IMAGE_INVALID_FORMAT;

    char namea[1024];
    sprintf(namea, "%S", name);

    auto tex = lod->GetTexture(lod->LoadTexture(namea, TEXTURE_16BIT_PALETTE));
    if (tex->pBits & 512)
        Log::Warning(L"Alpha texture is loaded as ColorKey (%s)", name);

    if (tex->pPalette16 && tex->paletted_pixels)
    {
        auto palette = tex->pPalette16;
        auto paletted_pixels = tex->paletted_pixels;

        auto width = tex->uTextureWidth;
        auto height = tex->uTextureHeight;
        auto pixels = new unsigned __int32[width * height];
        if (pixels)
        {
            for (unsigned int y = 0; y < height; ++y)
                for (unsigned int x = 0; x < width; ++x)
                {
                    auto pixel = tex->pPalette16[paletted_pixels[y * width + x]];
                    if (pixel == colorkey)
                        pixels[y * width + x] = 0x00000000;
                    else
                    {
                        extern unsigned __int32 Color32(unsigned __int16 color16);
                        pixels[y * width + x] = R5G6B5_to_A8R8G8B8(pixel, 255);
                    }
                }
            *out_width = width;
            *out_height = height;
            *out_pixels = pixels;
            *out_format = IMAGE_FORMAT_A8R8G8B8;
        }
    }

    return *out_pixels != nullptr;
}






struct PCX_Loader : public ImageLoader
{
    bool DecodePCX(const unsigned char *pcx_data, unsigned __int16 *pOutPixels, unsigned int *width, unsigned int *height);
};

bool PCX_Loader::DecodePCX(const unsigned char *pcx_data, unsigned __int16 *pOutPixels, unsigned int *width, unsigned int *height)
{
    unsigned char test_byte; // edx@3
    unsigned int read_offset; // ebx@37
    unsigned int row_position; // edi@40
    unsigned char value; // cl@63
    char count; // [sp+50h] [bp-Ch]@43
    unsigned short current_line; // [sp+54h] [bp-8h]@38
    unsigned short *dec_position;
    unsigned short *temp_dec_position;
    PCXHeader1 psx_head1;
    PCXHeader2 psx_head2;
    //	short int width, height;
    BYTE  color_map[48];	// Colormap for 16-color images


    memcpy(&psx_head1, pcx_data, 16);
    memcpy(&color_map, pcx_data + 16, 48);
    memcpy(&psx_head2, pcx_data + 64, 6);


    if (psx_head1.bpp != 8)
        return 3;
    *width = (short int)(psx_head1.right - psx_head1.left + 1);  // word @ 000014
    *height = (short int)(psx_head1.bottom - psx_head1.up + 1);  // word @ 000016


    unsigned int uNumPixels = *width * *height;		  // dword @ 000010

    memset(pOutPixels, 0, uNumPixels * sizeof(__int16));

    unsigned int r_mask = 0xF800;
    unsigned int num_r_bits = 5;
    unsigned int g_mask = 0x07E0;
    unsigned int num_g_bits = 6;
    unsigned int b_mask = 0x001F;
    unsigned int num_b_bits = 5;

    //При сохранении изображения подряд идущие пиксели одинакового цвета объединяются и вместо указания цвета для каждого пикселя
    //указывается цвет группы пикселей и их количество.
    read_offset = 128;
    if (psx_head2.planes != 3)
        return 0;
    current_line = 0;
    if (height > 0)
    {
        dec_position = pOutPixels;
        do
        {
            temp_dec_position = dec_position;
            row_position = 0;
            //decode red line
            if (psx_head2.pitch)
            {
                do
                {
                    test_byte = pcx_data[read_offset];
                    ++read_offset;
                    if ((test_byte & 0xC0) == 0xC0)//имеется ли объединение
                    {
                        value = pcx_data[read_offset];
                        ++read_offset;

                        if ((test_byte & 0x3F) > 0)
                        {
                            count = test_byte & 0x3F;//количество одинаковых пикселей
                            do
                            {
                                ++row_position;
                                //*temp_dec_position =0xFF000000;
                                //*temp_dec_position|=(unsigned long)value<<16;
                                *temp_dec_position |= r_mask & ((unsigned __int8)value << (num_g_bits + num_r_bits + num_b_bits - 8));
                                temp_dec_position++;
                                if (row_position == psx_head2.pitch)
                                    break;
                            } while (count-- != 1);
                        }
                    }
                    else
                    {
                        ++row_position;
                        //*temp_dec_position =0xFF000000; 
                        //*temp_dec_position|= (unsigned long)test_byte<<16;

                        *temp_dec_position |= r_mask & ((unsigned __int8)test_byte << (num_g_bits + num_r_bits + num_b_bits - 8));

                        temp_dec_position++;
                    }

                } while (row_position < psx_head2.pitch);
            }

            temp_dec_position = dec_position;
            row_position = 0;
            //decode green line
            while (row_position <  psx_head2.pitch)
            {
                test_byte = *(pcx_data + read_offset);
                ++read_offset;
                if ((test_byte & 0xC0) == 0xC0)
                {
                    value = *(pcx_data + read_offset);
                    ++read_offset;
                    if ((test_byte & 0x3F) > 0)
                    {
                        count = test_byte & 0x3F;
                        do
                        {
                            //*temp_dec_position|= (unsigned int)value<<8;
                            //temp_dec_position++;

                            *temp_dec_position |= g_mask & (unsigned __int16)((unsigned __int8)value << (num_g_bits + num_b_bits - 8));

                            temp_dec_position++;
                            ++row_position;
                            if (row_position == psx_head2.pitch)
                                break;

                        } while (count-- != 1);
                    }
                }
                else
                {
                    //*temp_dec_position |=(unsigned int) test_byte<<8;
                    //temp_dec_position++;

                    *temp_dec_position |= g_mask & (unsigned __int16)((unsigned __int8)test_byte << (num_g_bits + num_b_bits - 8));
                    temp_dec_position++;
                    ++row_position;
                }
            }

            temp_dec_position = dec_position;
            row_position = 0;
            //decode blue line
            while (row_position < psx_head2.pitch)
            {
                test_byte = *(pcx_data + read_offset);
                read_offset++;
                if ((test_byte & 0xC0) == 0xC0)
                {
                    value = *(pcx_data + read_offset);
                    ++read_offset;
                    if ((test_byte & 0x3F) > 0)
                    {
                        count = test_byte & 0x3F;
                        do
                        {
                            //*temp_dec_position|= value;
                            //temp_dec_position++;

                            *temp_dec_position |= value >> (8 - num_b_bits);
                            temp_dec_position++;

                            ++row_position;
                            if (row_position == psx_head2.pitch)
                                break;
                        } while (count-- != 1);
                    }
                }
                else
                {
                    //*temp_dec_position|= test_byte;
                    //temp_dec_position++;
                    *temp_dec_position |= test_byte >> (8 - num_b_bits);
                    temp_dec_position++;

                    ++row_position;
                }

            }
            ++current_line;
            dec_position += *width;
        } while (current_line < *height);
    }

    return true;
}




struct PCX_File_Loader : public PCX_Loader
{
    inline PCX_File_Loader(LODFile_IconsBitmaps *lod, const wchar_t *filename)
    {
        wcscpy(this->name, filename);
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);

    wchar_t               name[1024];
    LODFile_IconsBitmaps *lod;
};



bool PCX_File_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    FILE *file; // eax@1
    char color_map[48]; // [sp+Ch] [bp-98h]@7
    PCXHeader1 header1; // [sp+84h] [bp-20h]@7
    PCXHeader2 header2; // [sp+94h] [bp-10h]@7

    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    char namea[1024];
    sprintf(namea, "%S", name);

    file = fopen(namea, "rb");
    if (!file)
    {
        Log::Warning(L"Unable to load %s", name);
        return false;
    }

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto file_image = new unsigned char[filesize];
    fread(file_image, 1, filesize, file);
    fclose(file);


    memcpy(&header1, file_image, 0x10u);
    memcpy(color_map, file_image + 16, 0x30u);
    memcpy(&header2, file_image + 64, 6);
    if (header1.bpp != 8)
    {
        delete[] file_image;
        return false;
    }

    *width = header1.right - header1.left + 1;
    *height = header1.bottom - header1.up + 1;
    unsigned int num_pixels = *width * *height;
    *pixels = new unsigned short[num_pixels + 2];

    if (pixels)
    {
        if (!this->DecodePCX(file_image, (unsigned __int16 *)*pixels, width, height))
        {
            delete[] * pixels;
            *pixels = nullptr;
        }
        else
            *format = IMAGE_FORMAT_R5G6B5;
    }

    delete[] file_image;

    return *pixels != nullptr;
}




struct PCX_LOD_Loader : public PCX_Loader
{
    inline PCX_LOD_Loader(LOD::File *lod, const wchar_t *name)
    {
        wcscpy(this->name, name);
        this->lod = lod;
    }

    virtual bool Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format);

    wchar_t    name[1024];
    LOD::File *lod;
};



bool PCX_LOD_Loader::Load(unsigned int *width, unsigned int *height, void **pixels, IMAGE_FORMAT *format)
{
    FILE *file; // eax@1
    void *v6; // ebx@5
    char color_map[48]; // [sp+Ch] [bp-98h]@7
    Texture_MM7 DstBuf; // [sp+3Ch] [bp-68h]@1
    PCXHeader1 header1; // [sp+84h] [bp-20h]@7
    PCXHeader2 header2; // [sp+94h] [bp-10h]@7
    size_t Count; // [sp+A0h] [bp-4h]@4
    unsigned char *Str1a; // [sp+ACh] [bp+8h]@5

    *width = 0;
    *height = 0;
    *pixels = nullptr;
    *format = IMAGE_INVALID_FORMAT;

    char namea[1024];
    sprintf(namea, "%S", name);

    file = lod->FindContainer(namea, 0);
    if (!file)
    {
        Log::Warning(L"Unable to load %s", name);
        return false;
    }

    fread(&DstBuf, 1, 0x30u, file);
    Count = DstBuf.uTextureSize;
    if (DstBuf.uDecompressedSize)
    {
        Str1a = (unsigned char *)malloc(DstBuf.uDecompressedSize);
        v6 = malloc(DstBuf.uTextureSize);
        fread(v6, 1, Count, file);
        zlib::MemUnzip(Str1a, &DstBuf.uDecompressedSize, v6, DstBuf.uTextureSize);
        DstBuf.uTextureSize = DstBuf.uDecompressedSize;
        free(v6);
    }
    else
    {
        Str1a = (unsigned char *)malloc(DstBuf.uTextureSize);
        fread(Str1a, 1, Count, file);
    }

    memcpy(&header1, Str1a, 0x10u);
    memcpy(color_map, Str1a + 16, 0x30u);
    memcpy(&header2, Str1a + 64, 6);
    if (header1.bpp != 8)
    {
        free(Str1a);
        return false;
    }

    *width = header1.right - header1.left + 1;
    *height = header1.bottom - header1.up + 1;
    unsigned int num_pixels = *width * *height;
    *pixels = new unsigned short[num_pixels + 2];

    if (pixels)
    {
        if (!this->DecodePCX(Str1a, (unsigned __int16 *)*pixels, width, height))
        {
            delete[] *pixels;
            *pixels = nullptr;
        }
        else
            *format = IMAGE_FORMAT_R5G6B5;
    }

    free(Str1a);

    return *pixels != nullptr;
}






bool Image::PCX_From_IconsLOD(const wchar_t *name)
{
    loader = new PCX_LOD_Loader(pIcons_LOD, name);

    if (!lazy_initialization)
    {
        this->LoadImageData();
    }

    return true;
}

bool Image::PCX_From_NewLOD(const wchar_t *name)
{
    loader = new PCX_LOD_Loader(pNew_LOD, name);

    if (!lazy_initialization)
    {
        this->LoadImageData();
    }

    return true;
}

bool Image::PCX_From_File(const wchar_t *filename)
{
    loader = new PCX_File_Loader(pIcons_LOD, filename);

    if (!lazy_initialization)
    {
        this->LoadImageData();
    }

    return true;
}

bool Image::Alpha_From_LOD(const wchar_t *name)
{
    loader = new Alpha_LOD_Loader(pIcons_LOD, name);

    if (!lazy_initialization)
    {
        this->LoadImageData();
    }

    return true;
}

bool Image::Image16bit_From_LOD(const wchar_t *name)
{
    loader = new Image16bit_LOD_Loader(pIcons_LOD, name);

    if (!lazy_initialization)
    {
        this->LoadImageData();
    }

    return true;
}



bool Image::ColorKey_From_LOD(const wchar_t *name, unsigned __int16 colorkey)
{
    loader = new ColorKey_LOD_Loader(pIcons_LOD, name, colorkey);

    if (!lazy_initialization)
    {
        this->LoadImageData();
    }

    return true;
}

bool Image::LoadImageData()
{
    if (!this->initialized)
    {
        void *pixels;

        this->initialized = this->loader->Load(&width, &height, &pixels, &native_format);
        if (this->initialized && this->native_format != IMAGE_INVALID_FORMAT)
            this->pixels[native_format] = pixels;
    }

    return this->initialized;
}


unsigned int Image::GetWidth()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    if (this->initialized)
    {
        return this->width;
    }

    return 0;
}

unsigned int Image::GetHeight()
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    if (this->initialized)
    {
        return this->height;
    }
    return 0;
}

Image *Image::Create(unsigned int width, unsigned int height, IMAGE_FORMAT format, const void *pixels)
{
    auto img = new Image(false);
    if (img)
    {
        img->initialized = true;
        img->width = width;
        img->height = height;
        img->native_format = format;
        if (pixels)
        {
            unsigned int num_pixels = img->GetWidth() * img->GetHeight();
            unsigned int num_pixels_bytes = num_pixels * IMAGE_FORMAT_BytesPerPixel(format);
            img->pixels[format] = new unsigned char[num_pixels_bytes];

            memcpy(img->pixels[format], pixels, num_pixels_bytes);
        }
    }

    return img;
}

const void *Image::GetPixels(IMAGE_FORMAT format)
{
    if (!this->initialized)
    {
        this->LoadImageData();
    }

    if (initialized)
    {
        if (this->pixels[format])
        {
            return this->pixels[format];
        }

        auto native_pixels = this->pixels[this->native_format];
        if (native_pixels)
        {
            static ImageFormatConverter converters[IMAGE_NUM_FORMATS][IMAGE_NUM_FORMATS] =
            {
                // IMAGE_FORMAT_R5G6B5 ->
                {
                    nullptr, // IMAGE_FORMAT_R5G6B5
                    nullptr, // IMAGE_FORMAT_A1R5G5B5
                    nullptr  // IMAGE_FORMAT_A8R8G8B8
                },

                // IMAGE_FORMAT_A1R5G5B5 ->
                {
                    nullptr, // IMAGE_FORMAT_R5G6B5
                    nullptr, // IMAGE_FORMAT_A1R5G5B5
                    nullptr  // IMAGE_FORMAT_A8R8G8B8
                },

                // IMAGE_FORMAT_A8R8G8B8 ->
                {
                    Image_A8R8G8B8_to_R5G6B5, // IMAGE_FORMAT_R5G6B5
                    nullptr, // IMAGE_FORMAT_A1R5G5B5
                    nullptr  // IMAGE_FORMAT_A8R8G8B8
                },
            };

            ImageFormatConverter cvt = converters[this->native_format][format];
            if (cvt)
            {
                unsigned int num_pixels = this->GetWidth() * this->GetHeight();

                void *cvt_pixels = new unsigned char[num_pixels * IMAGE_FORMAT_BytesPerPixel(format)];
                if (cvt(width * height, native_pixels, cvt_pixels))
                {
                    return this->pixels[format] = cvt_pixels;
                }
                else
                {
                    delete[] cvt_pixels;
                    cvt_pixels = nullptr;
                }
            }
        }
    }
    return nullptr;
}


bool Image::Release()
{
    if (initialized)
    {
        if (loader)
        {
            delete loader;
            loader = nullptr;
        }

        for (unsigned int i = 0; i < IMAGE_NUM_FORMATS; ++i)
        {
            if (pixels[i])
            {
                delete[] pixels[i];
                pixels[i] = nullptr;
            }
        }

        native_format = IMAGE_INVALID_FORMAT;
        width = 0;
        height = 0;
    }

    delete this;
    return true;
}