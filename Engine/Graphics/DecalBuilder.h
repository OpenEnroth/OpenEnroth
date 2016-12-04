#pragma once

#include <cstdint>

#include "Render.h"

/*  158 */
#pragma pack(push, 1)
struct Bloodsplat
{
  //----- (0043B538) --------------------------------------------------------
  inline Bloodsplat()
  {
    this->field_1C = 0;
    this->field_20 = 0;
    //this->field_24 = 0;
    //this->vdestructor_ptr = &Bloodsplat_pvdtor;
  }

  //----- (0043B54C) --------------------------------------------------------
  //void  Bloodsplat::vdtor(Bloodsplat *this, char a2)

  //----- (0043B569) --------------------------------------------------------
  virtual ~Bloodsplat()
  {
  }

  //void ( ***vdestructor_ptr)(Bloodsplat *, bool);
  float x;
  float y;
  float z;
  float radius;
  float dot_dist;
  unsigned char r;
  unsigned char g;
  unsigned char b;
  char field_1B;
  int field_1C;
  unsigned long long field_20;
  //int field_24;
};
#pragma pack(pop)

/*  159 */
#pragma pack(push, 1)
struct BloodsplatContainer
{
  //----- (0043B688) --------------------------------------------------------
  inline BloodsplatContainer()
  {
  /*_eh_vector_constructor_iterator_(
    this->std__vector_pBloodsplats,
    40,
    64,
    (void ( *)(void *))Bloodsplat::Bloodsplat,
    (void ( *)(void *))Bloodsplat::dtor);
  v1->std__vector_pBloodsplats_size = 0;*/
    uNumBloodsplats = 0;
  }
  //----- (0043B6D6) --------------------------------------------------------
  virtual ~BloodsplatContainer()
  {
    //this->vdestructor_ptr = &BloodsplatContainer_pvdtor;
    //_eh_vector_destructor_iterator_(this->std__vector_pBloodsplats, 40, 64, Bloodsplat::dtor);
  }


  void AddBloodsplat(float x, float y, float z, float r, float g, float b, float radius, int a8, int a9);
  void AddBloodsplat(float x, float y, float z, float radius, unsigned char r, unsigned char g, unsigned char b);


  //void ( ***vdestructor_ptr)(BloodsplatContainer *, bool);
  Bloodsplat std__vector_pBloodsplats[64];
  unsigned int std__vector_pBloodsplats_size;
  int uNumBloodsplats;
};
#pragma pack(pop)

extern struct BloodsplatContainer *pBloodsplatContainer; // idb

#pragma pack(push, 1)
struct DecalBuilder_stru0
{
  double _43B570_get_color_mult_by_time();

  int field_0;
  int field_4;
  int field_8;
  int field_C;
  int field_10;
  int field_14;
  int field_18;
  int field_1C_flags;
  __int64 field_20_time;
};
#pragma pack(pop)



/*  181 */
#pragma pack(push, 1)
struct Decal
{
  void Decal_base_ctor();
  //----- (0043B60C) --------------------------------------------------------
  inline Decal()
  {
    Decal_base_ctor();
    field_C1C = 0;
  }

  //----- (0043B625) --------------------------------------------------------
  //void  Decal::vdtor(Decal *this, bool a2)
  //----- (0043B641) --------------------------------------------------------
  virtual ~Decal()
  {
  }

  //void ( ***vdestructor_ptr)(Decal *, bool);
  int uNumVertices;
  RenderVertexSoft pVertices[64];
  __int16 field_C08;
  __int16 field_C0A;
  __int16 field_C0C;
  __int16 field_C0E;
  uint32_t uColorMultiplier;
  int field_C14;
  DecalBuilder_stru0 *field_C18;
  int field_C1C;
};
#pragma pack(pop)

/*  180 */
#pragma pack(push, 1)
struct DecalBuilder
{
  //----- (0049B408) --------------------------------------------------------
  DecalBuilder()
  {
	  this->DecalsCount = 0;
	  this->curent_decal_id = 0;
	  for (uint i = 0; i < 256; ++i)
	  {
		this->pVertices[i].flt_2C = 0.0f;
	  }
  }

  //----- (0049B471) --------------------------------------------------------
  virtual ~DecalBuilder()
  {}


  void AddBloodsplat(float x, float y, float z, float r, float g, float b, float radius, int a8, int a9);
  void Reset(unsigned int bPreserveBloodsplats);
  char ApplyDecals(int light_level, char a3, struct stru154 *a4, int a5, struct RenderVertexSoft *a6, struct IndoorCameraD3D_Vec4 *a7, char a8, unsigned int uSectorID);
  char _49B790_build_decal_geometry(int a2, char a3, Bloodsplat *a4, int a5, float a6, unsigned int uColorMultiplier, float a8, struct stru314 *a9, signed int a10, struct RenderVertexSoft *a11, char uClipFlags);
  bool ApplyBloodsplatDecals_IndoorFace(unsigned int uFaceID);
  char ApplyDecals_OutdoorFace(ODMFace *pFace);
  bool _49BE8A(struct Polygon *a2, Vec3_float_ *a3, float *a4, struct RenderVertexSoft *a5, unsigned int uStripType, char a7);
  void DrawDecals(float z_bias);
  void DrawBloodsplats();
  void DrawDecalDebugOutlines();



  //void ( ***vdestructor_ptr)(DecalBuilder *, bool);
  Decal Decals[1024];
  unsigned int DecalsCount;
  int curent_decal_id;//field_308008
  RenderVertexSoft pVertices[256];
  int std__vector_30B00C[1024];
  int uNumDecals;
  float field_30C010;
  float field_30C014;
  float field_30C018;
  float field_30C01C;
  float field_30C020;
  float field_30C024;
  float field_30C028;
  float field_30C02C;
  float flt_30C030;
  float field_30C034;
};
#pragma pack(pop)

extern struct DecalBuilder *pDecalBuilder;





