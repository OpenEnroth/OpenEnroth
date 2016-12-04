#pragma once



/*  327 */
enum TILE_DESC_FLAGS
{
  TILE_DESC_BURNS = 0x1,
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
  TILE_DESC_SCROLL_RIGHT = 0x2000,
};

#pragma warning( push )
#pragma warning( disable: 4341 )
enum Tileset: signed __int16
{
  Tileset_Grass = 0,
  Tileset_Snow = 1,
  Tilset_Desert = 2,
  Tileset_CooledLava = 3,
  Tileset_Dirt = 4,
  Tileset_Water = 5,
  Tileset_Badlands = 6,
  Tileset_Swamp = 7,
  Tileset_8 = 8,
  Tileset_9 = 9,
  Tileset_RoadGrassCobble = 10,
  Tileset_NULL =-1
};
#pragma warning( pop )

/*   48 */
#pragma pack(push, 1)
struct TileDesc  //26
{
  char pTileName[16];
  unsigned __int16 uTileID;
  unsigned __int16 uBitmapID;
  Tileset          tileset;
  unsigned __int16 uSection;
  unsigned __int16 uAttributes;
};
#pragma pack(pop)

/*   49 */
#pragma pack(push, 1)
struct TileTable
{
  //----- (00487E13) --------------------------------------------------------
  TileTable()
  {
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

  signed int sNumTiles;
  struct TileDesc *pTiles;
};
#pragma pack(pop)
