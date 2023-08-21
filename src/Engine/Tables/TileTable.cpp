#include "TileTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#include "Engine/ErrorHandling.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineIocContainer.h"
#include "Library/Random/Random.h"
#include "Library/Logger/Logger.h"
#include "Utility/String.h"
#include "FrameTableInc.h"
#include "Engine/Tables/TileEnums.h"
#include "Library/Random/RandomEngine.h"

struct TileTable *pTileTable;

GraphicsImage *TileDesc::GetTexture() {
    if (!this->texture) {
        this->texture = assets->getBitmap(this->name);
    }
    return this->texture;
}


//----- (00487E3B) --------------------------------------------------------
TileDesc *TileTable::GetTileById(unsigned int uTileID) {
    return &tiles[uTileID];
}

//----- (00487E58) --------------------------------------------------------
void TileTable::InitializeTileset(Tileset tileset) {
    //for (int i = 0; i < sNumTiles; ++i) {
        //if (pTiles[i].tileset == tileset && !pTiles[i].name.empty()) {
            // pTiles[i].uBitmapID =
            // pBitmaps_LOD->LoadTexture(pTiles[i].pTileName); if
            // (pTiles[i].uBitmapID != -1)
            //    pBitmaps_LOD->pTextures[pTiles[i].uBitmapID].palette_id2 =
            //    pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[pTiles[i].uBitmapID].palette_id1);
        //}
    //}
}

//----- (00487ED6) --------------------------------------------------------
int TileTable::GetTileForTerrainType(signed int terrain_type, bool not_random) {
    int v5;  // edx@3
    int v6;  // edx@11

    if (not_random || terrain_type > 8) {
        return GetTileId(terrain_type, 0);
    }
    v5 = vrng->random(50);
    if (v5 < 20) {
        return GetTileId(terrain_type, 0);
    } else if (v5 < 30) {
        return GetTileId(terrain_type, 1);
    } else if (v5 < 40) {
        return GetTileId(terrain_type, 2);
    } else if (v5 < 48) {
        return GetTileId(terrain_type, 3);
    }
    v6 = vrng->random(8);
    return GetTileId(terrain_type, v6 + 4);
}

//----- (00487F84) --------------------------------------------------------
unsigned int TileTable::GetTileId(unsigned int uTerrainType, unsigned int uSection) {
    for (size_t i = 0; i < tiles.size(); ++i) {
        if ((tiles[i].tileset == uTerrainType) &&
            (tiles[i].uSection == uSection))
            return i;
    }
    return 0;
}

//----- (00488047) --------------------------------------------------------
int TileTable::FromFileTxt(const char *pFilename) {
    TileTable *v2;          // ebp@1
    FILE *v3;               // eax@1
    unsigned int v4;        // ebx@3
    void *v5;               // eax@9
    uint16_t v6;    // ax@14
    const char *v7;         // ST14_4@14
    uint16_t v8;    // ax@14
    const char *v9;         // esi@14
    const char *v35;        // esi@67
    int j;                  // edi@152
    const char *v72;        // esi@153
    FILE *i;                // [sp-10h] [bp-304h]@3
    FILE *File;             // [sp+4h] [bp-2F0h]@1
    FrameTableTxtLine v84;  // [sp+8h] [bp-2ECh]@4
    FrameTableTxtLine v85;  // [sp+84h] [bp-270h]@4
    char Buf[490];          // [sp+100h] [bp-1F4h]@4

    v2 = this;
    v3 = fopen(pFilename, "r");
    File = v3;
    if (!v3) Error("TileTable::load - Unable to open file: %s.");
    v4 = 0;
    for (i = v3; fgets(Buf, sizeof(Buf), i); i = File) {
        *strchr(Buf, 10) = 0;
        memcpy(&v84, txt_file_frametable_parser(Buf, &v85), sizeof(v84));
        if (v84.uPropCount && *v84.pProperties[0] != 47) ++v4;
    }
    v2->tiles.reserve(v4);
    v2->tiles.resize(0);
    fseek(File, 0, 0);

    while (fgets(Buf, sizeof(Buf), File)) {
        *strchr(Buf, 10) = 0;
        memcpy(&v84, txt_file_frametable_parser(Buf, &v85), sizeof(v84));
        if (v84.uPropCount) {
            if (*v84.pProperties[0] != 47) {
                TileDesc &tile = v2->tiles.emplace_back();

                // strcpy(tile.pTileName, v84.pProperties[0]);
                v6 = atoi(v84.pProperties[1]);
                v7 = v84.pProperties[2];
                tile.uTileID = v6;
                v8 = atoi(v7);
                v9 = v84.pProperties[3];
                // tile.uBitmapID = v8;

                if (iequals(v9, "TTtype_NULL")) {
                    tile.tileset = Tileset_NULL;
                } else if (iequals(v9, "TTtype_Start")) {
                    tile.tileset = Tileset_Start;
                } else if (iequals(v9, "TTtype_Grass")) {
                    tile.tileset = Tileset_Grass;
                } else if (iequals(v9, "TTtype_Snow")) {
                    tile.tileset = Tileset_Snow;
                } else if (iequals(v9, "TTtype_Sand")) {
                    tile.tileset = Tileset_Desert;
                } else if (iequals(v9, "TTtype_Volcano")) {
                    tile.tileset = Tileset_CooledLava;
                } else if (iequals(v9, "TTtype_Dirt")) {
                    tile.tileset = Tileset_Dirt;
                } else if (iequals(v9, "TTtype_Water")) {
                    tile.tileset = Tileset_Water;
                } else if (iequals(v9, "TTtype_Cracked")) {
                    tile.tileset = Tileset_Badlands;
                } else if (iequals(v9, "TTtype_Swamp")) {
                    tile.tileset = Tileset_Swamp;
                } else if (iequals(v9, "TTtype_Tropical")) {
                    tile.tileset = Tileset_Tropical;
                } else if (iequals(v9, "TTtype_City")) {
                    tile.tileset = Tileset_City;
                } else if (iequals(v9, "TTtype_RoadGrassCobble")) {
                    tile.tileset = Tileset_RoadGrassCobble;
                } else if (iequals(v9, "TTtype_RoadGrassDirt")) {
                    tile.tileset = Tileset_RoadGrassDirt;
                } else if (iequals(v9, "TTtype_RoadSnowCobble")) {
                    tile.tileset = Tileset_RoadSnowCobble;
                } else if (iequals(v9, "TTtype_RoadSnowDirt")) {
                    tile.tileset = Tileset_RoadSnowDirt;
                } else if (iequals(v9, "TTtype_RoadSandCobble")) {
                    tile.tileset = Tileset_RoadSandCobble;
                } else if (iequals(v9, "TTtype_RoadSandDirt")) {
                    tile.tileset = Tileset_RoadSandDirt;
                } else if (iequals(v9, "TTtype_RoadVolcanoCobble")) {
                    tile.tileset = Tileset_RoadVolcanoCobble;
                } else if (iequals(v9, "TTtype_RoadVolcanoDirt")) {
                    tile.tileset = Tileset_RoadVolcanoDirt;
                } else if (iequals(v9, "TTtype_RoadCrackedCobble")) {
                    tile.tileset = Tileset_RoadCrackedCobble;
                } else if (iequals(v9, "TTtype_RoadCrackedDirt")) {
                    tile.tileset = Tileset_RoadCrackedDirt;
                } else if (iequals(v9, "TTtype_RoadSwampCobble")) {
                    tile.tileset = Tileset_RoadSwampCobble;
                } else if (iequals(v9, "TTtype_RoadSwampDirt")) {
                    tile.tileset = Tileset_RoadSwampDir;
                } else if (iequals(v9, "TTtype_RoadTropicalCobble")) {
                    tile.tileset = Tileset_RoadTropicalCobble;
                } else if (iequals(v9, "TTtype_RoadTropicalDirt")) {
                    tile.tileset = Tileset_RoadTropicalDirt;
                } else if (iequals(v9, "TTtype_RoadCityStone")) {
                    tile.tileset = Tileset_RoadCityStone;
                } else {
                    logger->warning("Unknown tile type {}", v9);
                }

                v35 = v84.pProperties[4];

                // Default
                tile.uSection = 0;
                tile.uAttributes = 0;

                if (iequals(v35, "TTsect_NULL")) {
                    tile.uSection |= TILE_SECT_NULL;
                } else if (iequals(v35, "TTsect_Start")) {
                    tile.uSection |= TILE_SECT_Start;
                } else if (iequals(v35, "TTsect_Base1")) {
                    tile.uSection = TILE_SECT_Base1;
                } else if (iequals(v35, "TTsect_Base2")) {
                    tile.uSection = TILE_SECT_Base2_NS;
                } else if (iequals(v35, "TTsect_Base3")) {
                    tile.uSection = TILE_SECT_Base3_EW;
                } else if (iequals(v35, "TTsect_Base4")) {
                    tile.uSection = TILE_SECT_Base4_N_E;
                } else if (iequals(v35, "TTsect_Special1")) {
                    tile.uSection = TILE_SECT_Special1_N_W;
                } else if (iequals(v35, "TTsect_Special2")) {
                    tile.uSection = TILE_SECT_Special2_S_E;
                } else if (iequals(v35, "TTsect_Special3")) {
                    tile.uSection = TILE_SECT_Special3_S_W;
                } else if (iequals(v35, "TTsect_Special4")) {
                    tile.uSection = TILE_SECT_Special4_NS_E;
                } else if (iequals(v35, "TTsect_Special5")) {
                    tile.uSection = TILE_SECT_Special5_NS_W;
                } else if (iequals(v35, "TTsect_Special6")) {
                    tile.uSection = TILE_SECT_Special6_EW_N;
                } else if (iequals(v35, "TTsect_Special7")) {
                    tile.uSection = TILE_SECT_Special7_EW_S;
                } else if (iequals(v35, "TTsect_Special8")) {
                    tile.uSection = TILE_SECT_Special8_NCAP;
                } else if (iequals(v35, "TTsect_NE1")) {
                    tile.uSection = TILE_SECT_NE1_SE1_ECAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_NW1")) {
                    tile.uSection = TILE_SECT_NW1_SW1_WCAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_SE1")) {
                    tile.uSection = TILE_SECT_NE1_SE1_ECAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_SW1")) {
                    tile.uSection = TILE_SECT_NW1_SW1_WCAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_E1")) {
                    tile.uSection = TILE_SECT_E1_DS;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_W1")) {
                    tile.uSection = TILE_SECT_W1_DW;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_N1")) {
                    tile.uSection = TILE_SECT_N1_DE;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_S1")) {
                    tile.uSection = TILE_SECT_S1_DSW;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_XNE1")) {
                    tile.uSection = TILE_SECT_XNE1_XSE1_DNE;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_XNW1")) {
                    tile.uSection = TILE_SECT_XNW1_XSW1_DNW;
                    tile.uAttributes = TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_XSE1")) {
                    tile.uSection = TILE_SECT_XNE1_XSE1_DNE;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_XSW1")) {
                    tile.uSection = TILE_SECT_XNW1_XSW1_DNW;
                    tile.uAttributes = TILE_DESC_TRANSITION;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_CROS")) {
                    tile.uSection = TILE_SECT_Base1;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_NS")) {
                    tile.uSection = TILE_SECT_Base2_NS;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_EW")) {
                    tile.uSection = TILE_SECT_Base3_EW;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_N_E")) {
                    tile.uSection = TILE_SECT_Base4_N_E;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_N_W")) {
                    tile.uSection = TILE_SECT_Special1_N_W;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_S_E")) {
                    tile.uSection = TILE_SECT_Special2_S_E;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_S_W")) {
                    tile.uSection = TILE_SECT_Special3_S_W;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_NS_E")) {
                    tile.uSection = TILE_SECT_Special4_NS_E;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_NS_W")) {
                    tile.uSection = TILE_SECT_Special5_NS_W;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_EW_N")) {
                    tile.uSection = TILE_SECT_Special6_EW_N;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_EW_S")) {
                    tile.uSection = TILE_SECT_Special7_EW_S;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_NCAP")) {
                    tile.uSection = TILE_SECT_Special8_NCAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_ECAP")) {
                    tile.uSection = TILE_SECT_NE1_SE1_ECAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_SCAP")) {
                    tile.uSection = TILE_SECT_SCAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_WCAP")) {
                    tile.uSection = TILE_SECT_NW1_SW1_WCAP;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DN")) {
                    tile.uSection = TILE_SECT_DN;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DS")) {
                    tile.uSection = TILE_SECT_E1_DS;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DW")) {
                    tile.uSection = TILE_SECT_W1_DW;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DE")) {
                    tile.uSection = TILE_SECT_N1_DE;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DSW")) {
                    tile.uSection = TILE_SECT_S1_DSW;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DNE")) {
                    tile.uSection = TILE_SECT_XNE1_XSE1_DNE;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DSE")) {
                    tile.uSection = TILE_SECT_DSE;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                } else if (iequals(v35, "TTsect_DNW")) {
                    tile.uSection = TILE_SECT_XNW1_XSW1_DNW;
                    tile.uAttributes |= TILE_DESC_TRANSITION;
                }

                for (j = 5; j < v84.uPropCount; ++j) {
                    v72 = v84.pProperties[j];
                    if (iequals(v84.pProperties[j], "TTattr_Burn")) {
                        tile.uAttributes |= TILE_DESC_BURN;
                    } else if (iequals(v72, "TTattr_Water")) {
                        tile.uAttributes |= TILE_DESC_WATER;
                    } else if (iequals(v72, "TTattr_Water2")) {
                        tile.uAttributes |= TILE_DESC_WATER_2;
                    } else if (iequals(v72, "TTattr_Block")) {
                        tile.uAttributes |= TILE_DESC_BLOCK;
                    } else if (iequals(v72, "TTattr_Repulse")) {
                        tile.uAttributes |= TILE_DESC_REPULSE;
                    } else if (iequals(v72, "TTattr_Flat")) {
                        tile.uAttributes |= TILE_DESC_FLAT;
                    } else if (iequals(v72, "TTattr_Wave")) {
                        tile.uAttributes |= TILE_DESC_WAVY;
                    } else if (iequals(v72, "TTattr_NoDraw")) {
                        tile.uAttributes |= TILE_DESC_DONT_DRAW;
                    } else if (iequals(v72, "TTattr_Transition")) {
                        tile.uAttributes |= TILE_DESC_TRANSITION;
                    } else {
                        logger->warning("Unknown tile attribute {}", v72);
                    }
                }
            }
        }
    }

    fclose(File);
    return 1;
}
