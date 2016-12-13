#pragma

#include "Indoor.h"
#include "../Tables/TileFrameTable.h"
#include "BSPModel.h"

#define DAY_ATTRIB_FOG  1

/*  256 */
#pragma pack(push, 1)
struct ODMHeader
{
  int uVersion;
  char pMagic[4];
  unsigned int uCompressedSize;
  unsigned int uDecompressedSize;
};
#pragma pack(pop)



/*   78 */
#pragma pack(push, 1)
struct OutdoorLocationTileType
{
  Tileset tileset;
  unsigned __int16 uTileID;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DMap
    {
    unsigned __int8 field0;
    unsigned __int8 field1;
    };
#pragma pack(pop)

/*   79 */
#pragma pack(push, 1)
struct OutdoorLocationTerrain
{
  //----- (0047C794) --------------------------------------------------------
  inline OutdoorLocationTerrain()
  {
    pHeightmap = nullptr;
    pTilemap = nullptr;
    pAttributemap = nullptr;
    pDmap = nullptr;;
    this->field_10 = 0;
    this->field_12 = 0;

    
  }

  void _47C7A9();
  void Release();
  void FillDMap(int X, int Y, int W, int Z);
  int _47CB57(unsigned char *pixels_8bit, int a2, int num_pixels);
  bool ZeroLandscape();
  bool Initialize();


  unsigned __int8 *pHeightmap;
  unsigned __int8 *pTilemap;
  unsigned __int8 *pAttributemap;
  struct DMap *pDmap;
  __int16 field_10;
  __int16 field_12;
  __int16 field_14;
  __int16 field_16;
  int field_18;
  int field_1C;
};
#pragma pack(pop)


/*   81 */
#pragma pack(push, 1)
struct ODMFace
{
  bool HasEventHint();

  
  static bool IsBackfaceNotCulled(struct RenderVertexSoft *a2, struct Polygon *polygon);
  
  inline bool Invisible() const {return (uAttributes & FACE_INVISIBLE) != 0;}
  inline bool Visible() const   {return !Invisible();}
  inline bool Portal() const    {return (uAttributes & FACE_PORTAL) != 0;}
  inline bool Fluid() const     {return (uAttributes & FACE_FLUID) != 0;}
  inline bool Indoor_sky() const {return (uAttributes & FACE_INDOOR_SKY) != 0;}
  inline bool Clickable() const {return (uAttributes & FACE_CLICKABLE) != 0;}
  inline bool Pressure_Plate() const {return (uAttributes & FACE_PRESSURE_PLATE) != 0;}
  inline bool Ethereal() const {return (uAttributes & FACE_ETHEREAL) != 0;}

  struct Plane_int_ pFacePlane;
  int zCalc1;
  int zCalc2;
  int zCalc3;
  unsigned int uAttributes;
  unsigned __int16 pVertexIDs[20];
  unsigned __int16 pTextureUIDs[20];
  unsigned __int16 pTextureVIDs[20];
  signed __int16 pXInterceptDisplacements[20];
  signed __int16 pYInterceptDisplacements[20];
  signed __int16 pZInterceptDisplacements[20];
  __int16 uTextureID;
  __int16 sTextureDeltaU;
  __int16 sTextureDeltaV;
  struct BBox_short_ pBoundingBox;
  __int16 sCogNumber;
  __int16 sCogTriggeredID;
  __int16 sCogTriggerType;
  char field_128;
  char field_129;
  unsigned __int8 uGradientVertex1;
  unsigned __int8 uGradientVertex2;
  unsigned __int8 uGradientVertex3;
  unsigned __int8 uGradientVertex4;
  unsigned __int8 uNumVertices;
  unsigned __int8 uPolygonType;
  unsigned __int8 uShadeType;
  unsigned __int8 bVisible;
  char field_132;
  char field_133;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct OutdoorLocation
{
  OutdoorLocation();
  void subconstuctor();
  //int New_SKY_NIGHT_ID;
  void ExecDraw(unsigned int bRedraw);
  void PrepareActorsDrawList();
  void CreateDebugLocation();
  void Release();
  bool Load(const char *pFilename, ODMFace *File, size_t a4, int *thisa);
  int GetTileIdByTileMapId(signed int a2);
  unsigned int DoGetTileTexture(signed int uX, signed int uZ);
  int _47ED83(signed int a2, signed int a3);
  int ActuallyGetSomeOtherTileInfo(signed int uX, signed int uY);
  int DoGetHeightOnTerrain(signed int sX, signed int sZ);
  int GetSoundIdByPosition(signed int X_pos, signed int Y_pos, int a4);
  int UpdateDiscoveredArea(int a2, int a3, int a4);
  bool IsMapCellFullyRevealed(signed int a2, signed int a3);
  bool IsMapCellPartiallyRevealed(signed int a2, signed int a3);
  bool _47F0E2();
  bool PrepareDecorations();
  void ArrangeSpriteObjects();
  bool InitalizeActors(int a1);
  bool LoadRoadTileset();
  bool LoadTileGroupIds();
  double GetFogDensityByTime();
  int GetSomeOtherTileInfo(int sX, int sY);
  unsigned int GetTileTexture(int sX, int sZ);
  int GetHeightOnTerrain(int sX, int sZ);
  bool Initialize(const char *pFilename, int File, size_t uRespawnInterval, int *thisa);
  //bool Release2();
  bool GetTravelDestination(signed int sPartyX, signed int sPartyZ, char *pOut, signed int a5);
  void MessWithLUN();
  void UpdateSunlightVectors();
  void UpdateFog();
  int GetNumFoodRequiredToRestInCurrentPos(int x, signed int y, int z);
  void SetFog();
  void Draw();

  static void LoadActualSkyFrame();


  char pLevelFilename[32];
  char pLocationFileName[32];
  char pLocationFileDescription[32];
  char pSkyTextureName[32];
  char pGroundTileset[32];
  OutdoorLocationTileType pTileTypes[4];    // [3]  road tileset
  int uNumBModels;
  struct OutdoorLocationTerrain pTerrain;
  void *pCmap;
  BSPModel *pBModels;
  unsigned int numFaceIDListElems;
  unsigned __int16 *pFaceIDLIST;
  unsigned int *pOMAP;
  signed int sSky_TextureID;
  signed int sMainTile_BitmapID;
  __int16 field_F0;
  __int16 field_F2;
  int field_F4;
  char field_F8[968];
  unsigned int uNumSpawnPoints;
  struct SpawnPointMM7 *pSpawnPoints;
  struct DDM_DLV_Header ddm;
  LocationTime_stru1 loc_time;
  //unsigned __int64 uLastVisitDay;
  //char sky_texture_name[12];
  //int day_attrib;
  //int day_fogrange_1;
  //int day_fogrange_2;
 // char field_510[24];
  unsigned char uFullyRevealedCellOnMap[88][11];//968         the inner array is 11 bytes long, because every bit is used for a separate cell, so in the end it's 11 * 8 bits = 88 values
  unsigned char uPartiallyRevealedCellOnMap[88][11];//[968]
  int field_CB8;
  int max_terrain_dimming_level;
  int field_CC0;
  unsigned int pSpriteIDs_LUN[8];
  unsigned int uSpriteID_LUNFULL;
  int field_CE8;
  unsigned int uSpriteID_LUN3_4_cp;
  int field_CF0;
  unsigned int uSpriteID_LUN1_2_cp;
  int field_CF8;
  unsigned int uSpriteID_LUN1_4_cp;
  int field_D00;
  unsigned __int16 uSpriteID_LUN_SUN;
  __int16 field_D06;
  int field_D08;
  int field_D0C;
  int field_D10;
  int field_D14;
  int inv_sunlight_x;
  int inv_sunlight_y;
  int inv_sunlight_z;
  int field_D24;
  int field_D28;
  int field_D2C;
  Vec3_int_ vSunlight;
  int field_D3C;
  int field_D40;
  int field_D44;
  int field_D48;
  int field_D4C;
  float field_D50;
  int field_D54;
  int field_D58;
  int field_D5C;
  int field_D60;
  int field_D64;
  char field_D68[111900];
  float fFogDensity;
  int uLastSunlightUpdateMinute;
};
#pragma pack(pop)


extern struct OutdoorLocation *pOutdoor;

void ODM_UpdateUserInputAndOther();
int ODM_GetFloorLevel(int X, signed int Y, int Z, int, int *pOnWater, int *bmodel_pid, int bWaterWalk);
int GetCeilingHeight(int Party_X, signed int Party_Y, int Party_ZHeight, int pFaceID);
void ODM_GetTerrainNormalAt(int pos_x, int pos_z, Vec3_int_ *out);
void UpdateActors_ODM();
void ODM_ProcessPartyActions();
char Is_out15odm_underwater();
void SetUnderwaterFog();
void ODM_Project(unsigned int uNumVertices);
void sub_487DA9();
void ODM_LoadAndInitialize(const char *pLevelFilename, struct ODMRenderParams *thisa);
unsigned int GetLevelFogColor();
int sub_47C3D7_get_fog_specular(int a1, int a2, float a3);
unsigned int WorldPosToGridCellX(int); // weak
unsigned int WorldPosToGridCellZ(int); // weak
int GridCellToWorldPosX(int); // weak
int GridCellToWorldPosZ(int); // weak
void sub_481ED9_MessWithODMRenderParams();
bool IsTerrainSlopeTooHigh(int pos_x, int pos_y);
int GetTerrainHeightsAroundParty2(int a1, int a2, int *a3, int a4);









