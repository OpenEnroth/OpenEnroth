#pragma once


/*  127 */
#pragma pack(push, 1)
struct stru10
{
  stru10();
  virtual ~stru10();
  char _49C5DA(struct BLVFace *pFace, struct RenderVertexSoft *pVertices, unsigned int *pNumVertices, struct IndoorCameraD3D_Vec4 *a5, struct RenderVertexSoft *pOutBounding);
  bool CalcPortalShape(struct BLVFace *pFace, struct IndoorCameraD3D_Vec4 *pPortalDataFrustum, struct RenderVertexSoft *pOutBounding);
  char _49C720(struct RenderVertexSoft *pFaceBounding, struct IndoorCameraD3D_Vec4 *pPortalDataFrustum);
  bool FindFaceNormal(struct RenderVertexSoft *pFaceBounding1, struct RenderVertexSoft *pFaceBounding2, struct Vec3_float_ *pRayStart, struct IndoorCameraD3D_Vec4 *pPortalDataFrustum);
  bool FindFacePlane(struct RenderVertexSoft *face, struct Vec3_float_ *out_normal, float *out_distance);
  bool CalcFaceBounding(struct BLVFace *pFace, struct RenderVertexSoft *pFaceLimits, unsigned int uNumVertices, struct RenderVertexSoft *pOutBounding);
  void CalcPolygonLimits(struct BLVFace *pFace, struct RenderVertexSoft pOutVertices[4]);
  void _49CE9E(struct BLVFace *pFace, struct RenderVertexSoft *pVertices, unsigned int uNumVertices, RenderVertexSoft *pOutLimits);


  void ( ***vdestructor_ptr)(stru10 *, bool);
  int bDoNotDrawPortalFrustum;
};
#pragma pack(pop)
