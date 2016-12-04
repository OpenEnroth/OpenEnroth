#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Engine/Engine.h"

#include "TileFrameTable.h"
#include "FrameTableInc.h"
#include "Engine/Graphics/PaletteManager.h"
#include "../LOD.h"


//----- (00487E1D) --------------------------------------------------------
TileTable::~TileTable()
{
  free(pTiles);
  pTiles = nullptr;
  sNumTiles = 0;
}

//----- (00487E3B) --------------------------------------------------------
TileDesc *TileTable::GetTileById(unsigned int uTileID)
{
  Assert(uTileID < sNumTiles);
  return &pTiles[uTileID];
}

//----- (00487E58) --------------------------------------------------------
void TileTable::InitializeTileset(Tileset tileset)
{
  for (int i = 0; i < sNumTiles; ++i )
  {
    if (pTiles[i].tileset == tileset && pTiles[i].pTileName[0] )
    {
      pTiles[i].uBitmapID = pBitmaps_LOD->LoadTexture(pTiles[i].pTileName);
      if ( pTiles[i].uBitmapID != -1 )
        pBitmaps_LOD->pTextures[pTiles[i].uBitmapID].palette_id2 = pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[pTiles[i].uBitmapID].palette_id1);
    }
  }
}

//----- (00487ED6) --------------------------------------------------------
int TileTable::GetTileForTerrainType( signed int terrain_type, bool not_random )
    {
  int v5; // edx@3
  int v6; // edx@11

  if (  not_random || terrain_type > 8 )
  {
    return GetTileId(terrain_type, 0);
  }
   v5 = rand() % 50;
  if ( v5 < 20)
  {
    return GetTileId(terrain_type, 0);
  }
  else if ( v5 < 30 )
  {
    return GetTileId(terrain_type, 1);
  }
  else if ( v5 < 40 )
  {
    return GetTileId(terrain_type, 2);
  }
  else if ( v5 < 48 )
  {
    return GetTileId(terrain_type, 3);
  }
  v6 = rand() % 8;
  return GetTileId(terrain_type, v6+4);
  return 0;
}

//----- (00487F84) --------------------------------------------------------
unsigned int TileTable::GetTileId(unsigned int uTerrainType, unsigned int uSection)
{  
  for (int i=0; i<sNumTiles; ++i)
      {
      if( (pTiles[i].tileset==uTerrainType)&&(pTiles[i].uSection==uSection))
          return i;
      }
  return 0;
}

//----- (00487FB4) --------------------------------------------------------
void TileTable::ToFile()
{
  TileTable *v1; // esi@1
  FILE *v2; // eax@1
  FILE *v3; // edi@1

  TileTable* Str = this;

  v1 = Str;
  v2 = fopen("data\\dtile.bin", "wb");
  v3 = v2;
  if ( !v2 )
    Error("Unable to save dtile.bin!");
  fwrite(v1, 4u, 1u, v2);
  fwrite(v1->pTiles, 0x1Au, v1->sNumTiles, v3);
  fclose(v3);
}

//----- (00488000) --------------------------------------------------------
void TileTable::FromFile(void *data_mm6, void *data_mm7, void *data_mm8)
{
  uint num_mm6_tiles = data_mm6 ? *(int *)data_mm6 : 0,
       num_mm7_tiles = data_mm7 ? *(int *)data_mm7 : 0,
       num_mm8_tiles = data_mm8 ? *(int *)data_mm8 : 0;
  sNumTiles = num_mm6_tiles + num_mm7_tiles + num_mm8_tiles;
  Assert(sNumTiles);
  Assert(!num_mm8_tiles);

  pTiles = (TileDesc *)malloc(sNumTiles * sizeof(TileDesc));
  memcpy(pTiles,                                 (char *)data_mm7 + 4, num_mm7_tiles * sizeof(TileDesc));
  memcpy(pTiles + num_mm7_tiles,                 (char *)data_mm6 + 4, num_mm6_tiles * sizeof(TileDesc));
  memcpy(pTiles + num_mm6_tiles + num_mm7_tiles, (char *)data_mm8 + 4, num_mm8_tiles * sizeof(TileDesc));
}

//----- (00488047) --------------------------------------------------------
int TileTable::FromFileTxt(const char *pFilename)
{
  TileTable *v2; // ebp@1
  FILE *v3; // eax@1
  unsigned int v4; // ebx@3
  void *v5; // eax@9
  unsigned __int16 v6; // ax@14
  const char *v7; // ST14_4@14
  unsigned __int16 v8; // ax@14
  const char *v9; // esi@14
  int v10; // eax@17
  int v11; // eax@20
  int v12; // eax@22
  int v13; // eax@24
  int v14; // eax@26
  int v15; // eax@28
  int v16; // eax@30
  int v17; // eax@32
  int v18; // eax@34
  int v19; // eax@36
  int v20; // eax@38
  int v21; // eax@40
  int v22; // eax@42
  int v23; // eax@44
  int v24; // eax@46
  int v25; // eax@48
  int v26; // eax@50
  int v27; // eax@52
  int v28; // eax@54
  int v29; // eax@56
  int v30; // eax@58
  int v31; // eax@60
  int v32; // eax@62
  int v33; // eax@64
  int v34; // eax@66
  const char *v35; // esi@67
  int v36; // eax@70
  int v37; // eax@73
  int v38; // eax@75
  int v39; // eax@77
  int v40; // eax@79
  int v41; // eax@81
  int v42; // eax@83
  int v43; // eax@85
  int v44; // eax@87
  int v45; // eax@89
  int v46; // eax@91
  int v47; // eax@93
  int v48; // eax@108
  int v49; // eax@110
  int v50; // eax@112
  int v51; // eax@114
  int v52; // eax@116
  int v53; // eax@118
  int v54; // eax@120
  int v55; // eax@122
  int v56; // eax@124
  int v57; // eax@126
  int v58; // eax@128
  int v59; // eax@130
  int v60; // eax@132
  int v61; // eax@134
  int v62; // eax@136
  int v63; // eax@138
  int v64; // eax@140
  int v65; // eax@142
  int v66; // eax@144
  int v67; // eax@146
  int v68; // eax@148
  int v69; // eax@150
  int v70; // eax@151
  int j; // edi@152
  const char *v72; // esi@153
  int v73; // eax@154
  int v74; // eax@156
  int v75; // eax@160
  int v76; // eax@162
  int v77; // eax@164
  int v78; // eax@166
  int v79; // eax@168
  int v80; // eax@170
  FILE *i; // [sp-10h] [bp-304h]@3
  FILE *File; // [sp+4h] [bp-2F0h]@1
  FrameTableTxtLine v84; // [sp+8h] [bp-2ECh]@4
  FrameTableTxtLine v85; // [sp+84h] [bp-270h]@4
  char Buf; // [sp+100h] [bp-1F4h]@4

  v2 = this;
  v3 = fopen(pFilename, "r");
  File = v3;
  if ( !v3 )
    Error("TileTable::load - Unable to open file: %s.");
  v4 = 0;
  for ( i = v3; fgets(&Buf, 490, i); i = File )
  {
    *strchr(&Buf, 10) = 0;
    memcpy(&v84, txt_file_frametable_parser(&Buf, &v85), sizeof(v84));
    if ( v84.uPropCount && *v84.pProperties[0] != 47 )
      ++v4;
  }
  v2->sNumTiles = v4;
  v5 = malloc(26 * v4);
  v2->pTiles = (TileDesc *)v5;
  if ( !v5 )
    Error("TileTable::Load - Out of Memory!");
  memset(v5, 0, 26 * v2->sNumTiles);
  v2->sNumTiles = 0;
  fseek(File, 0, 0);
  if ( fgets(&Buf, 490, File) )
  {
    while ( 1 )
    {
      *strchr(&Buf, 10) = 0;
      memcpy(&v84, txt_file_frametable_parser(&Buf, &v85), sizeof(v84));
      if ( v84.uPropCount )
      {
        if ( *v84.pProperties[0] != 47 )
          break;
      }
LABEL_173:
      if ( !fgets(&Buf, 490, File) )
        goto LABEL_174;
    }
    strcpy(v2->pTiles[v2->sNumTiles].pTileName, v84.pProperties[0]);
    v6 = atoi(v84.pProperties[1]);
    v7 = v84.pProperties[2];
    v2->pTiles[v2->sNumTiles].uTileID = v6;
    v8 = atoi(v7);
    v9 = v84.pProperties[3];
    v2->pTiles[v2->sNumTiles].uBitmapID = v8;
    v2->pTiles[v2->sNumTiles].tileset = Tileset_Grass;
    if ( _stricmp(v9, "TTtype_NULL") )
    {
      if ( _stricmp(v9, "TTtype_Start") )
      {
        if ( _stricmp(v9, "TTtype_Grass") )
        {
          if ( _stricmp(v9, "TTtype_Cracked") )
          {
            if ( _stricmp(v9, "TTtype_Snow") )
            {
              if ( _stricmp(v9, "TTtype_Sand") )
              {
                if ( _stricmp(v9, "TTtype_Volcano") )
                {
                  if ( _stricmp(v9, "TTtype_Dirt") )
                  {
                    if ( _stricmp(v9, "TTtype_Water") )
                    {
                      if ( _stricmp(v9, "TTtype_Tropical") )
                      {
                        if ( _stricmp(v9, "TTtype_Swamp") )
                        {
                          if ( _stricmp(v9, "TTtype_City") )
                          {
                            if ( _stricmp(v9, "TTtype_RoadGrassCobble") )
                            {
                              if ( _stricmp(v9, "TTtype_RoadGrassDirt") )
                              {
                                if ( _stricmp(v9, "TTtype_RoadCrackedCobble") )
                                {
                                  if ( _stricmp(v9, "TTtype_RoadCrackedDirt") )
                                  {
                                    if ( _stricmp(v9, "TTtype_RoadSandCobble") )
                                    {
                                      if ( _stricmp(v9, "TTtype_RoadSandDirt") )
                                      {
                                        if ( _stricmp(v9, "TTtype_RoadVolcanoCobble") )
                                        {
                                          if ( _stricmp(v9, "TTtype_RoadVolcanoDirt") )
                                          {
                                            if ( _stricmp(v9, "TTtype_RoadSwampCobble") )
                                            {
                                              if ( _stricmp(v9, "TTtype_RoadSwampDirt") )
                                              {
                                                if ( _stricmp(v9, "TTtype_RoadTropicalCobble") )
                                                {
                                                  if ( _stricmp(v9, "TTtype_RoadTropicalDirt") )
                                                  {
                                                    if ( _stricmp(v9, "TTtype_RoadSnowCobble") )
                                                    {
                                                      if ( _stricmp(v9, "TTtype_RoadSnowDirt") )
                                                      {
                                                        if ( !_stricmp(v9, "TTtype_RoadCityStone") )
                                                        {
                                                          v34 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                                          *(char *)v34 |= 0x1Cu;
                                                        }
                                                      }
                                                      else
                                                      {
                                                        v33 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                                        *(char *)v33 |= 0xDu;
                                                      }
                                                    }
                                                    else
                                                    {
                                                      v32 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                                      *(char *)v32 |= 0xCu;
                                                    }
                                                  }
                                                  else
                                                  {
                                                    v31 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                                    *(char *)v31 |= 0x1Bu;
                                                  }
                                                }
                                                else
                                                {
                                                  v30 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                                  *(char *)v30 |= 0x1Au;
                                                }
                                              }
                                              else
                                              {
                                                v29 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                                *(char *)v29 |= 0x19u;
                                              }
                                            }
                                            else
                                            {
                                              v28 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                              *(char *)v28 |= 0x18u;
                                            }
                                          }
                                          else
                                          {
                                            v27 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                            *(char *)v27 |= 0x11u;
                                          }
                                        }
                                        else
                                        {
                                          v26 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                          *(char *)v26 |= 0x10u;
                                        }
                                      }
                                      else
                                      {
                                        v25 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                        *(char *)v25 |= 0xFu;
                                      }
                                    }
                                    else
                                    {
                                      v24 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                      *(char *)v24 |= 0xEu;
                                    }
                                  }
                                  else
                                  {
                                    v23 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                    *(char *)v23 |= 0x17u;
                                  }
                                }
                                else
                                {
                                  v22 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                  *(char *)v22 |= 0x16u;
                                }
                              }
                              else
                              {
                                v21 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                                *(char *)v21 |= 0xBu;
                              }
                            }
                            else
                            {
                              v20 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                              *(char *)v20 |= 0xAu;
                            }
                          }
                          else
                          {
                            v19 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                            *(char *)v19 |= 9u;
                          }
                        }
                        else
                        {
                          v18 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                          *(char *)v18 |= 7u;
                        }
                      }
                      else
                      {
                        v17 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                        *(char *)v17 |= 8u;
                      }
                    }
                    else
                    {
                      v16 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                      *(char *)v16 |= 5u;
                    }
                  }
                  else
                  {
                    v15 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                    *(char *)v15 |= 4u;
                  }
                }
                else
                {
                  v14 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                  *(char *)v14 |= 3u;
                }
              }
              else
              {
                v13 = (int)&v2->pTiles[v2->sNumTiles].tileset;
                *(char *)v13 |= 2u;
              }
            }
            else
            {
              v12 = (int)&v2->pTiles[v2->sNumTiles].tileset;
              *(char *)v12 |= 1u;
            }
          }
          else
          {
            v11 = (int)&v2->pTiles[v2->sNumTiles].tileset;
            *(char *)v11 |= 6u;
          }
        }
      }
      else
      {
        v10 = (int)&v2->pTiles[v2->sNumTiles].uSection;
        *(char *)v10 |= 0xFEu;
      }
    }
    else
    {
      LOBYTE(v2->pTiles[v2->sNumTiles].tileset) = -1;
    }
    v35 = v84.pProperties[4];
    v2->pTiles[v2->sNumTiles].uSection = 0;
    v2->pTiles[v2->sNumTiles].uAttributes = 0;
    if ( !_stricmp(v35, "TTsect_NULL") )
    {
      LOBYTE(v2->pTiles[v2->sNumTiles].uSection) = -1;
LABEL_152:
      for ( j = 5; j < v84.uPropCount; ++j )
      {
        v72 = v84.pProperties[j];
        if ( _stricmp(v84.pProperties[j], "TTattr_Burn") )
        {
          if ( _stricmp(v72, "TTattr_Water") )
          {
            if ( _stricmp(v72, "TTattr_Water2") )
            {
              if ( _stricmp(v72, "TTattr_Block") )
              {
                if ( _stricmp(v72, "TTattr_Repulse") )
                {
                  if ( _stricmp(v72, "TTattr_Flat") )
                  {
                    if ( _stricmp(v72, "TTattr_Wave") )
                    {
                      if ( _stricmp(v72, "TTattr_NoDraw") )
                      {
                        if ( !_stricmp(v72, "TTattr_Transition") )
                        {
                          v80 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
                          *(short *)v80 |= 0x200u;
                        }
                      }
                      else
                      {
                        v79 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
                        *(char *)v79 |= 0x40u;
                      }
                    }
                    else
                    {
                      v78 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
                      *(char *)v78 |= 0x20u;
                    }
                  }
                  else
                  {
                    v77 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
                    *(char *)v77 |= 0x10u;
                  }
                }
                else
                {
                  v76 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
                  *(char *)v76 |= 8u;
                }
              }
              else
              {
                v75 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
                *(char *)v75 |= 4;
              }
            }
            else
              HIBYTE(v2->pTiles[v2->sNumTiles].uAttributes) |= 1;
          }
          else
          {
            v74 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
            *(char *)v74 |= 2;
          }
        }
        else
        {
          v73 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
          *(char *)v73 |= 1u;
        }
      }
      ++v2->sNumTiles;
      goto LABEL_173;
    }
    if ( !_stricmp(v35, "TTsect_Start") )
    {
      v36 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v36 |= 0xFEu;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Base1") )
      goto LABEL_152;
    if ( !_stricmp(v35, "TTsect_Base2") )
    {
      v37 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v37 |= 1u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Base3") )
    {
      v38 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v38 |= 2u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Base4") )
    {
      v39 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v39 |= 3u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special1") )
    {
      v40 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v40 |= 4u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special2") )
    {
      v41 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v41 |= 5u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special3") )
    {
      v42 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v42 |= 6u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special4") )
    {
      v43 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v43 |= 7u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special5") )
    {
      v44 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v44 |= 8u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special6") )
    {
      v45 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v45 |= 9u;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special7") )
    {
      v46 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v46 |= 0xAu;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_Special8") )
    {
      v47 = (int)&v2->pTiles[v2->sNumTiles].uSection;
      *(char *)v47 |= 0xBu;
      goto LABEL_152;
    }
    if ( !_stricmp(v35, "TTsect_NE1") )
      goto LABEL_130;
    if ( !_stricmp(v35, "TTsect_NW1") )
      goto LABEL_134;
    if ( !_stricmp(v35, "TTsect_SE1") )
      goto LABEL_130;
    if ( !_stricmp(v35, "TTsect_SW1") )
      goto LABEL_134;
    if ( !_stricmp(v35, "TTsect_E1") )
      goto LABEL_138;
    if ( !_stricmp(v35, "TTsect_W1") )
      goto LABEL_140;
    if ( !_stricmp(v35, "TTsect_N1") )
      goto LABEL_142;
    if ( !_stricmp(v35, "TTsect_S1") )
      goto LABEL_144;
    if ( !_stricmp(v35, "TTsect_XNE1") )
      goto LABEL_146;
    if ( _stricmp(v35, "TTsect_XNW1") )
    {
      if ( !_stricmp(v35, "TTsect_XSE1") )
        goto LABEL_146;
      if ( _stricmp(v35, "TTsect_XSW1") )
      {
        if ( !_stricmp(v35, "TTsect_CROS") )
        {
LABEL_151:
          v70 = (int)&v2->pTiles[v2->sNumTiles].uAttributes;
          *(short *)v70 |= 0x200u;
          goto LABEL_152;
        }
        if ( !_stricmp(v35, "TTsect_NS") )
        {
          v48 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v48 |= 1u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_EW") )
        {
          v49 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v49 |= 2u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_N_E") )
        {
          v50 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v50 |= 3u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_N_W") )
        {
          v51 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v51 |= 4u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_S_E") )
        {
          v52 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v52 |= 5u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_S_W") )
        {
          v53 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v53 |= 6u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_NS_E") )
        {
          v54 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v54 |= 7u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_NS_W") )
        {
          v55 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v55 |= 8u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_EW_N") )
        {
          v56 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v56 |= 9u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_EW_S") )
        {
          v57 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v57 |= 0xAu;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_NCAP") )
        {
          v58 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v58 |= 0xBu;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_ECAP") )
        {
LABEL_130:
          v59 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v59 |= 0xCu;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_SCAP") )
        {
          v60 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v60 |= 0xDu;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_WCAP") )
        {
LABEL_134:
          v61 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v61 |= 0xEu;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DN") )
        {
          v62 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v62 |= 0xFu;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DS") )
        {
LABEL_138:
          v63 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v63 |= 0x10u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DW") )
        {
LABEL_140:
          v64 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v64 |= 0x11u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DE") )
        {
LABEL_142:
          v65 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v65 |= 0x12u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DSW") )
        {
LABEL_144:
          v66 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v66 |= 0x13u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DNE") )
        {
LABEL_146:
          v67 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v67 |= 0x14u;
          goto LABEL_151;
        }
        if ( !_stricmp(v35, "TTsect_DSE") )
        {
          v68 = (int)&v2->pTiles[v2->sNumTiles].uSection;
          *(char *)v68 |= 0x15u;
          goto LABEL_151;
        }
        if ( _stricmp(v35, "TTsect_DNW") )
          goto LABEL_152;
      }
    }
    v69 = (int)&v2->pTiles[v2->sNumTiles].uSection;
    *(char *)v69 |= 0x16u;
    goto LABEL_151;
  }
LABEL_174:
  fclose(File);
  return 1;
}

