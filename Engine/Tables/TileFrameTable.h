#pragma once

#include "Engine/Strings.h"
#include "Engine/AssetsManager.h"

/*  327 */
enum TILE_DESC_FLAGS {
    TILE_DESC_NULL = 0,
    TILE_DESC_BURN = 0x1,
    TILE_DESC_WATER = 0x2,
    TILE_DESC_BLOCK = 0x4,
    TILE_DESC_REPULSE = 0x8,
    TILE_DESC_FLAT = 0x10,
    TILE_DESC_WAVY = 0x20,
    TILE_DESC_DONT_DRAW = 0x40,
    TILE_DESC_WATER_2 = 0x100,
    TILE_DESC_TRANSITION = 0x200,
    TILE_DESC_SCROLL_DOWN = 0x400,
    TILE_DESC_SCROLL_UP = 0x800,
    TILE_DESC_SCROLL_LEFT = 0x1000,
    TILE_DESC_SCROLL_RIGHT = 0x2000
};

enum TILE_SECT_FLAGS {
    TILE_SECT_NULL = -1,
    TILE_SECT_Start = -2,
    TILE_SECT_Base1 = 0,
    TILE_SECT_Base2_NS = 1,
    TILE_SECT_Base3_EW = 2,
    TILE_SECT_Base4_N_E = 3,
    TILE_SECT_Special1_N_W = 4,
    TILE_SECT_Special2_S_E = 5,
    TILE_SECT_Special3_S_W = 6,
    TILE_SECT_Special4_NS_E = 7,
    TILE_SECT_Special5_NS_W = 8,
    TILE_SECT_Special6_EW_N = 9,
    TILE_SECT_Special7_EW_S = 0xA,
    TILE_SECT_Special8_NCAP = 0xB,
    TILE_SECT_NE1_SE1_ECAP = 0xC,
    TILE_SECT_SCAP = 0xD,
    TILE_SECT_NW1_SW1_WCAP = 0xE,
    TILE_SECT_DN = 0xF,
    TILE_SECT_E1_DS = 0x10,
    TILE_SECT_W1_DW = 0x11,
    TILE_SECT_N1_DE = 0x12,
    TILE_SECT_S1_DSW = 0x13,
    TILE_SECT_XNE1_XSE1_DNE = 0x14,
    TILE_SECT_DSE = 0x15,
    TILE_SECT_XNW1_XSW1_DNW = 0x16
};

#pragma warning(push)
#pragma warning(disable : 4341)
enum Tileset : int16_t {
    Tileset_Grass = 0,
    Tileset_Snow = 1,
    Tileset_Desert = 2,
    Tileset_CooledLava = 3,
    Tileset_Dirt = 4,
    Tileset_Water = 5,
    Tileset_Badlands = 6,
    Tileset_Swamp = 7,
    Tileset_Tropical = 8,
    Tileset_City = 9,
    Tileset_RoadGrassCobble = 10,
    Tileset_RoadGrassDirt = 11,
    Tileset_RoadSnowCobble = 12,
    Tileset_RoadSnowDirt = 13,
    Tileset_RoadSandCobble = 14,
    Tileset_RoadSandDirt = 15,
    Tileset_RoadVolcanoCobble = 16,
    Tileset_RoadVolcanoDirt = 17,
    Tileset_RoadCrackedCobble = 22,
    Tileset_RoadCrackedDirt = 23,
    Tileset_RoadSwampCobble = 24,
    Tileset_RoadSwampDir = 25,
    Tileset_RoadTropicalCobble = 26,
    Tileset_RoadTropicalDirt = 27,
    Tileset_RoadCityStone = 28,
    Tileset_NULL = -1,
    Tileset_Start = -2
};
#pragma warning(pop)

class Texture;

class TileDesc {
 public:
    inline TileDesc() : texture(nullptr) {}

    String name = "";
    uint16_t uTileID = 0;
    Tileset tileset = Tileset_NULL;
    uint16_t uSection = 0;
    uint16_t uAttributes = TILE_DESC_NULL;

    inline Texture *GetTexture() {
        if (!this->texture) {
            this->texture = assets->GetBitmap(this->name);
        }
        return this->texture;
    }

    // inline bool IsWaterTile() const         { return this->name == "wtrtyl";
    // }
    inline bool IsWaterTile() const { return this->uAttributes & 2; }
    inline bool IsWaterBorderTile() const {
        return this->name.find("wtrdr", 0) == 0 ||
               this->name.find("hwtrdr") == 0;
    }

 protected:
    Texture *texture;
};

/*   49 */
#pragma pack(push, 1)
struct TileTable {
    //----- (00487E13) --------------------------------------------------------
    TileTable() {
        this->pTiles = nullptr;
        this->sNumTiles = 0;
    }
    ~TileTable();

    TileDesc *GetTileById(unsigned int uTileID);
    void InitializeTileset(Tileset tileset);
    int GetTileForTerrainType(signed int a1, bool a2);
    unsigned int GetTileId(unsigned int uTerrainType, unsigned int uSection);
    void ToFile();
    void FromFile(void *data_mm6, void *data_mm7, void *data_mm8);
    int FromFileTxt(const char *pFilename);

    int sNumTiles;
    TileDesc *pTiles;
};
#pragma pack(pop)
