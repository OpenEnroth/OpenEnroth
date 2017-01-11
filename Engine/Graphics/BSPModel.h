#pragma once
#include "../VectorTypes.h"


#pragma pack(push, 1)
struct BSPNode //8
{
    __int16 uFront;
    __int16 uBack;
    __int16 uCoplanarOffset;
    __int16 uCoplanarSize;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct BSPVertexBuffer
{
    int uNumVertices;
    Vec3_int_ *pVertices;
};
#pragma pack(pop)



/*   80 */
#pragma pack(push, 1)
struct BSPModel
{
    void Release();


    char pModelName[32];
    char pModelName2[32];
    int field_40;
    struct BSPVertexBuffer pVertices;
    int uNumFaces;
    unsigned int uNumConvexFaces;
    struct ODMFace *pFaces;
    unsigned __int16 *pFacesOrdering;
    unsigned int uNumNodes;
    struct BSPNode *pNodes;
    unsigned int uNumDecorations;
    int sCenterX;
    int sCenterY;
    Vec3_int_ vPosition;
    int sMinX;
    int sMinY;
    int sMinZ;
    int sMaxX;
    int sMaxY;
    int sMaxZ;
    int sSomeOtherMinX;
    int sSomeOtherMinY;
    int sSomeOtherMinZ;
    int sSomeOtherMaxX;
    int sSomeOtherMaxY;
    int sSomeOtherMaxZ;
    Vec3_int_ vBoundingCenter;
    int sBoundingRadius;
};
#pragma pack(pop)
