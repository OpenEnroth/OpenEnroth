#pragma once

#include "../VectorTypes.h"

#pragma pack(push, 1)
struct BSPNode {
  int16_t uFront;
  int16_t uBack;
  int16_t uCoplanarOffset;
  int16_t uCoplanarSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BSPVertexBufferData {
  uint32_t uNumVertices;
  uint32_t pVertices;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BSPVertexBuffer {
  uint32_t uNumVertices;
  Vec3_int_ *pVertices;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BSPModelData {
  char pModelName[32];
  char pModelName2[32];
  int32_t field_40;
  struct BSPVertexBufferData ppVertices;
  uint32_t uNumFaces;
  uint32_t uNumConvexFaces;
  uint32_t ppFaces;
  uint32_t ppFacesOrdering;
  uint32_t uNumNodes;
  uint32_t ppNodes;
  uint32_t uNumDecorations;
  int32_t sCenterX;
  int32_t sCenterY;
  Vec3_int_ vPosition;
  int32_t sMinX;
  int32_t sMinY;
  int32_t sMinZ;
  int32_t sMaxX;
  int32_t sMaxY;
  int32_t sMaxZ;
  int32_t sSomeOtherMinX;
  int32_t sSomeOtherMinY;
  int32_t sSomeOtherMinZ;
  int32_t sSomeOtherMaxX;
  int32_t sSomeOtherMaxY;
  int32_t sSomeOtherMaxZ;
  Vec3_int_ vBoundingCenter;
  int32_t sBoundingRadius;
};
#pragma pack(pop)

struct BSPModel : public BSPModelData {
  void Release();
  void Load(uint8_t *data);

  struct BSPVertexBuffer pVertices;
  struct ODMFace *pFaces;
  uint16_t *pFacesOrdering;
  struct BSPNode *pNodes;
};
