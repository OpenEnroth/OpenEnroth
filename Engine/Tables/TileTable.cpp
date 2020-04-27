#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/PaletteManager.h"

#include "Engine/Serialization/LegacyImages.h"

#include "Platform/Api.h"

#include "FrameTableInc.h"
#include "TileFrameTable.h"


//----- (00487E1D) --------------------------------------------------------
TileTable::~TileTable() {
    free(pTiles);
    pTiles = nullptr;
    sNumTiles = 0;
}

//----- (00487E3B) --------------------------------------------------------
TileDesc *TileTable::GetTileById(unsigned int uTileID) {
    Assert(uTileID < sNumTiles);
    return &pTiles[uTileID];
}

//----- (00487E58) --------------------------------------------------------
void TileTable::InitializeTileset(Tileset tileset) {
    for (int i = 0; i < sNumTiles; ++i) {
        if (pTiles[i].tileset == tileset && !pTiles[i].name.empty()) {
            // pTiles[i].uBitmapID =
            // pBitmaps_LOD->LoadTexture(pTiles[i].pTileName); if
            // (pTiles[i].uBitmapID != -1)
            //    pBitmaps_LOD->pTextures[pTiles[i].uBitmapID].palette_id2 =
            //    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[pTiles[i].uBitmapID].palette_id1);
        }
    }
}

//----- (00487ED6) --------------------------------------------------------
int TileTable::GetTileForTerrainType(signed int terrain_type, bool not_random) {
    int v5;  // edx@3
    int v6;  // edx@11

    if (not_random || terrain_type > 8) {
        return GetTileId(terrain_type, 0);
    }
    v5 = rand() % 50;
    if (v5 < 20) {
        return GetTileId(terrain_type, 0);
    } else if (v5 < 30) {
        return GetTileId(terrain_type, 1);
    } else if (v5 < 40) {
        return GetTileId(terrain_type, 2);
    } else if (v5 < 48) {
        return GetTileId(terrain_type, 3);
    }
    v6 = rand() % 8;
    return GetTileId(terrain_type, v6 + 4);
}

//----- (00487F84) --------------------------------------------------------
unsigned int TileTable::GetTileId(unsigned int uTerrainType,
                                  unsigned int uSection) {
    for (int i = 0; i < sNumTiles; ++i) {
        if ((pTiles[i].tileset == uTerrainType) &&
            (pTiles[i].uSection == uSection))
            return i;
    }
    return 0;
}

//----- (00487FB4) --------------------------------------------------------
void TileTable::ToFile() {
    TileTable *v1;  // esi@1
    FILE *v2;       // eax@1
    FILE *v3;       // edi@1

    TileTable *Str = this;

    v1 = Str;
    v2 = fcaseopen("data/dtile.bin", "wb");
    v3 = v2;
    if (!v2) Error("Unable to save dtile.bin!");
    fwrite(v1, 4u, 1u, v2);
    fwrite(v1->pTiles, 0x1Au, v1->sNumTiles, v3);
    fclose(v3);
}

//----- (00488000) --------------------------------------------------------
void TileTable::FromFile(void *data_mm6, void *data_mm7, void *data_mm8) {
    uint num_mm6_tiles = data_mm6 ? *(int *)data_mm6 : 0,
         num_mm7_tiles = data_mm7 ? *(int *)data_mm7 : 0,
         num_mm8_tiles = data_mm8 ? *(int *)data_mm8 : 0;
    this->sNumTiles = num_mm7_tiles /*+ num_mm6_tiles + num_mm8_tiles*/;
    Assert(sNumTiles);

    auto tiles = new TileDesc[sNumTiles];
    auto tile_data = (TileDesc_MM7 *)((unsigned char *)data_mm7 + 4);
    for (unsigned int i = 0; i < num_mm7_tiles; ++i) {
        tiles[i].name = tile_data->pTileName;
        std::transform(tiles[i].name.begin(), tiles[i].name.end(),
                       tiles[i].name.begin(), ::tolower);
        if (tiles[i].name.find("wtrdr") == 0) {
            tiles[i].name.insert(
                tiles[i].name.begin(),
                'h');  // mm7 uses hd water tiles with legacy names
        }

        tiles[i].uTileID = tile_data->uTileID;
        tiles[i].tileset = (Tileset)tile_data->tileset;
        tiles[i].uSection = tile_data->uSection;
        tiles[i].uAttributes = tile_data->uAttributes;

        tile_data++;
    }
    // pTiles = (TileDesc *)malloc(sNumTiles * sizeof(TileDesc));
    // memcpy(pTiles, (char *)data_mm7 + 4, num_mm7_tiles * sizeof(TileDesc));
    // memcpy(pTiles + num_mm7_tiles, (char *)data_mm6 + 4, num_mm6_tiles *
    // sizeof(TileDesc)); memcpy(pTiles + num_mm6_tiles + num_mm7_tiles, (char
    // *)data_mm8 + 4, num_mm8_tiles * sizeof(TileDesc));

    this->pTiles = tiles;
}

//----- (00488047) --------------------------------------------------------
int TileTable::FromFileTxt(const char *pFilename) {
    TileTable *v2;          // ebp@1
    FILE *v3;               // eax@1
    unsigned int v4;        // ebx@3
    void *v5;               // eax@9
    unsigned __int16 v6;    // ax@14
    const char *v7;         // ST14_4@14
    unsigned __int16 v8;    // ax@14
    const char *v9;         // esi@14
    const char *v35;        // esi@67
    int j;                  // edi@152
    const char *v72;        // esi@153
    FILE *i;                // [sp-10h] [bp-304h]@3
    FILE *File;             // [sp+4h] [bp-2F0h]@1
    FrameTableTxtLine v84;  // [sp+8h] [bp-2ECh]@4
    FrameTableTxtLine v85;  // [sp+84h] [bp-270h]@4
    char Buf;               // [sp+100h] [bp-1F4h]@4

    v2 = this;
    v3 = fcaseopen(pFilename, "r");
    File = v3;
    if (!v3) Error("TileTable::load - Unable to open file: %s.");
    v4 = 0;
    for (i = v3; fgets(&Buf, 490, i); i = File) {
        *strchr(&Buf, 10) = 0;
        memcpy(&v84, txt_file_frametable_parser(&Buf, &v85), sizeof(v84));
        if (v84.uPropCount && *v84.pProperties[0] != 47) ++v4;
    }
    v2->sNumTiles = v4;
    v5 = malloc(sizeof(TileDesc) * v4);
    v2->pTiles = (TileDesc *)v5;
    if (!v5) Error("TileTable::Load - Out of Memory!");
    memset(v5, 0, sizeof(TileDesc) * v2->sNumTiles);
    v2->sNumTiles = 0;
    fseek(File, 0, 0);

    while (fgets(&Buf, 490, File)) {
        *strchr(&Buf, 10) = 0;
        memcpy(&v84, txt_file_frametable_parser(&Buf, &v85), sizeof(v84));
        if (v84.uPropCount) {
            if (*v84.pProperties[0] != 47) {
                // strcpy(v2->pTiles[v2->sNumTiles].pTileName, v84.pProperties[0]);
                v6 = atoi(v84.pProperties[1]);
                v7 = v84.pProperties[2];
                v2->pTiles[v2->sNumTiles].uTileID = v6;
                v8 = atoi(v7);
                v9 = v84.pProperties[3];
                // v2->pTiles[v2->sNumTiles].uBitmapID = v8;

                if (!_stricmp(v9, "TTtype_NULL")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_NULL;
                } else if (!_stricmp(v9, "TTtype_Start")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Start;
                } else if (!_stricmp(v9, "TTtype_Grass")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Grass;
                } else if (!_stricmp(v9, "TTtype_Snow")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Snow;
                } else if (!_stricmp(v9, "TTtype_Sand")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Desert;
                } else if (!_stricmp(v9, "TTtype_Volcano")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_CooledLava;
                } else if (!_stricmp(v9, "TTtype_Dirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Dirt;
                } else if (!_stricmp(v9, "TTtype_Water")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Water;
                } else if (!_stricmp(v9, "TTtype_Cracked")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Badlands;
                } else if (!_stricmp(v9, "TTtype_Swamp")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Swamp;
                } else if (!_stricmp(v9, "TTtype_Tropical")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_Tropical;
                } else if (!_stricmp(v9, "TTtype_City")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_City;
                } else if (!_stricmp(v9, "TTtype_RoadGrassCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadGrassCobble;
                } else if (!_stricmp(v9, "TTtype_RoadGrassDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadGrassDirt;
                } else if (!_stricmp(v9, "TTtype_RoadSnowCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadSnowCobble;
                } else if (!_stricmp(v9, "TTtype_RoadSnowDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadSnowDirt;
                } else if (!_stricmp(v9, "TTtype_RoadSandCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadSandCobble;
                } else if (!_stricmp(v9, "TTtype_RoadSandDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadSandDirt;
                } else if (!_stricmp(v9, "TTtype_RoadVolcanoCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadVolcanoCobble;
                } else if (!_stricmp(v9, "TTtype_RoadVolcanoDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadVolcanoDirt;
                } else if (!_stricmp(v9, "TTtype_RoadCrackedCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadCrackedCobble;
                } else if (!_stricmp(v9, "TTtype_RoadCrackedDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadCrackedDirt;
                } else if (!_stricmp(v9, "TTtype_RoadSwampCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadSwampCobble;
                } else if (!_stricmp(v9, "TTtype_RoadSwampDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadSwampDir;
                } else if (!_stricmp(v9, "TTtype_RoadTropicalCobble")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadTropicalCobble;
                } else if (!_stricmp(v9, "TTtype_RoadTropicalDirt")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadTropicalDirt;
                } else if (!_stricmp(v9, "TTtype_RoadCityStone")) {
                    v2->pTiles[v2->sNumTiles].tileset = Tileset_RoadCityStone;
                } else {
                    logger->Warning("%s", StringPrintf("Unknown tile type %s", v9).c_str());
                }

                v35 = v84.pProperties[4];

                // Default
                v2->pTiles[v2->sNumTiles].uSection = 0;
                v2->pTiles[v2->sNumTiles].uAttributes = 0;

                if (!_stricmp(v35, "TTsect_NULL")) {
                    v2->pTiles[v2->sNumTiles].uSection |= TILE_SECT_NULL;
                } else if (!_stricmp(v35, "TTsect_Start")) {
                    v2->pTiles[v2->sNumTiles].uSection |= TILE_SECT_Start;
                } else if (!_stricmp(v35, "TTsect_Base1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base1;
                } else if (!_stricmp(v35, "TTsect_Base2")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base2_NS;
                } else if (!_stricmp(v35, "TTsect_Base3")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base3_EW;
                } else if (!_stricmp(v35, "TTsect_Base4")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base4_N_E;
                } else if (!_stricmp(v35, "TTsect_Special1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special1_N_W;
                } else if (!_stricmp(v35, "TTsect_Special2")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special2_S_E;
                } else if (!_stricmp(v35, "TTsect_Special3")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special3_S_W;
                } else if (!_stricmp(v35, "TTsect_Special4")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special4_NS_E;
                } else if (!_stricmp(v35, "TTsect_Special5")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special5_NS_W;
                } else if (!_stricmp(v35, "TTsect_Special6")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special6_EW_N;
                } else if (!_stricmp(v35, "TTsect_Special7")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special7_EW_S;
                } else if (!_stricmp(v35, "TTsect_Special8")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special8_NCAP;
                } else if (!_stricmp(v35, "TTsect_NE1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_NE1_SE1_ECAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_NW1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_NW1_SW1_WCAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_SE1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_NE1_SE1_ECAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_SW1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_NW1_SW1_WCAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_E1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_E1_DS;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_W1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_W1_DW;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_N1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_N1_DE;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_S1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_S1_DSW;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_XNE1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_XNE1_XSE1_DNE;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_XNW1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_XNW1_XSW1_DNW;
                    v2->pTiles[v2->sNumTiles].uAttributes = TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_XSE1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_XNE1_XSE1_DNE;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_XSW1")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_XNW1_XSW1_DNW;
                    v2->pTiles[v2->sNumTiles].uAttributes = TILE_DESC_TRANSITION;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_CROS")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base1;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_NS")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base2_NS;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_EW")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base3_EW;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_N_E")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Base4_N_E;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_N_W")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special1_N_W;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_S_E")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special2_S_E;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_S_W")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special3_S_W;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_NS_E")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special4_NS_E;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_NS_W")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special5_NS_W;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_EW_N")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special6_EW_N;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_EW_S")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special7_EW_S;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_NCAP")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_Special8_NCAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_ECAP")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_NE1_SE1_ECAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_SCAP")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_SCAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_WCAP")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_NW1_SW1_WCAP;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DN")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_DN;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                }
                if (!_stricmp(v35, "TTsect_DS")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_E1_DS;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DW")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_W1_DW;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DE")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_N1_DE;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DSW")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_S1_DSW;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DNE")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_XNE1_XSE1_DNE;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DSE")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_DSE;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                } else if (!_stricmp(v35, "TTsect_DNW")) {
                    v2->pTiles[v2->sNumTiles].uSection = TILE_SECT_XNW1_XSW1_DNW;
                    v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                }

                for (j = 5; j < v84.uPropCount; ++j) {
                    v72 = v84.pProperties[j];
                    if (!_stricmp(v84.pProperties[j], "TTattr_Burn")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_BURN;
                    } else if (!_stricmp(v72, "TTattr_Water")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_WATER;
                    } else if (!_stricmp(v72, "TTattr_Water2")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_WATER_2;
                    } else if (!_stricmp(v72, "TTattr_Block")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_BLOCK;
                    } else if (!_stricmp(v72, "TTattr_Repulse")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_REPULSE;
                    } else if (!_stricmp(v72, "TTattr_Flat")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_FLAT;
                    } else if (!_stricmp(v72, "TTattr_Wave")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_WAVY;
                    } else if (!_stricmp(v72, "TTattr_NoDraw")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_DONT_DRAW;
                    } else if (!_stricmp(v72, "TTattr_Transition")) {
                        v2->pTiles[v2->sNumTiles].uAttributes |= TILE_DESC_TRANSITION;
                    } else {
                        logger->Warning("%s", StringPrintf("Unknown tile attribute %s", v72).c_str());
                    }
                }
                ++v2->sNumTiles;
            }
        }
    }

    fclose(File);
    return 1;
}
