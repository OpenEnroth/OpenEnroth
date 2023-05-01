#include "CompositeImages.h"

#include <string>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/LOD.h"

#include "Serializer.h"
#include "Deserializer.h"
#include "CommonImages.h"

void Deserialize(const IndoorLocation_MM7 &src, IndoorLocation *dst) {
    Deserialize(src.header, &dst->blv);
    Deserialize(src.vertices, &dst->pVertices);
    Deserialize(src.faces, &dst->pFaces);
    Deserialize(src.faceData, &dst->pLFaces);

    for (size_t i = 0, j = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];

        pFace->pVertexIDs = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        pFace->pXInterceptDisplacements = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        pFace->pYInterceptDisplacements = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        pFace->pZInterceptDisplacements = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        pFace->pVertexUIDs = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        pFace->pVertexVIDs = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        assert(j <= dst->pLFaces.size());
    }

    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];

        std::string texName;
        Deserialize(src.faceTextures[i], &texName);
        pFace->SetTexture(texName);
    }

    Deserialize(src.faceExtras, &dst->pFaceExtras);

    for (uint i = 0; i < dst->pFaceExtras.size(); ++i) {
        std::string texName;
        Deserialize(src.faceExtraTextures[i], &texName);

        if (texName.empty())
            dst->pFaceExtras[i].uAdditionalBitmapID = -1;
        else
            dst->pFaceExtras[i].uAdditionalBitmapID = pBitmaps_LOD->LoadTexture(texName);
    }

    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];
        BLVFaceExtra *pFaceExtra = &dst->pFaceExtras[pFace->uFaceExtraID];

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventHint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    Deserialize(src.sectors, &dst->pSectors);
    Deserialize(src.sectorData, &dst->ptr_0002B0_sector_rdata);

    for (size_t i = 0, j = 0; i < dst->pSectors.size(); ++i) {
        BLVSector *pSector = &dst->pSectors[i];

        pSector->pFloors = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumFloors;

        pSector->pWalls = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumWalls;

        pSector->pCeilings = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumCeilings;

        pSector->pFluids = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumFluids;

        pSector->pPortals = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumPortals;

        pSector->pFaceIDs = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumFaces;

        pSector->pCogs = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumCogs;

        pSector->pDecorationIDs = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumDecorations;

        pSector->pMarkers = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumMarkers;

        assert(j <= dst->ptr_0002B0_sector_rdata.size());
    }

    Deserialize(src.sectorLightData, &dst->ptr_0002B8_sector_lrdata);

    for (uint i = 0, j = 0; i < dst->pSectors.size(); ++i) {
        BLVSector *pSector = &dst->pSectors[i];

        pSector->pLights = dst->ptr_0002B8_sector_lrdata.data() + j;
        j += pSector->uNumLights;

        assert(j <= dst->ptr_0002B8_sector_lrdata.size());
    }

    Deserialize(src.decorations, &pLevelDecorations);

    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        std::string name;
        Deserialize(src.decorationNames[i], &name);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(name);
    }

    Deserialize(src.lights, &dst->pLights);
    Deserialize(src.bspNodes, &dst->pNodes);
    Deserialize(src.spawnPoints, &dst->pSpawnPoints);
    Deserialize(src.mapOutlines, &dst->pMapOutlines);
}

void Deserialize(const Blob &src, IndoorLocation_MM7 *dst, std::function<void()> progress) {
    BlobDeserializer stream(src);
    progress();
    stream.ReadRaw(&dst->header);
    stream.ReadVector(&dst->vertices);
    progress();
    progress();
    stream.ReadVector(&dst->faces);
    stream.ReadSizedVector(&dst->faceData, dst->header.uFaces_fdata_Size / sizeof(uint16_t));
    progress();
    stream.ReadSizedVector(&dst->faceTextures, dst->faces.size());
    progress();
    stream.ReadVector(&dst->faceExtras);
    progress();
    stream.ReadSizedVector(&dst->faceExtraTextures, dst->faceExtras.size());
    progress();
    stream.ReadVector(&dst->sectors);
    progress();
    stream.ReadSizedVector(&dst->sectorData, dst->header.uSector_rdata_Size / sizeof(uint16_t));
    stream.ReadSizedVector(&dst->sectorLightData, dst->header.uSector_lrdata_Size / sizeof(uint16_t));
    progress();
    progress();
    stream.ReadRaw(&dst->doorCount);
    progress();
    progress();
    stream.ReadVector(&dst->decorations);
    stream.ReadSizedVector(&dst->decorationNames, dst->decorations.size());
    progress();
    stream.ReadVector(&dst->lights);
    progress();
    progress();
    stream.ReadVector(&dst->bspNodes);
    progress();
    progress();
    stream.ReadVector(&dst->spawnPoints);
    progress();
    progress();
    stream.ReadVector(&dst->mapOutlines);
}

void Serialize(const IndoorLocation &src, IndoorSave_MM7 *dst) {
}

void Deserialize(const IndoorSave_MM7 &src, IndoorLocation *dst) {
    Deserialize(src.header, &dst->dlv);
    Deserialize(src.visibleOutlines, &dst->_visible_outlines);

    for (size_t i = 0; i < dst->pMapOutlines.size(); ++i) {
        BLVMapOutline *pVertex = &dst->pMapOutlines[i];
        if ((uint8_t)(1 << (7 - i % 8)) & dst->_visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];
        BLVFaceExtra *pFaceExtra = &dst->pFaceExtras[pFace->uFaceExtraID];

        pFace->uAttributes = FaceAttributes(src.faceAttributes[i]);

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventHint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    Deserialize(src.actors, &pActors);

    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    Deserialize(src.spriteObjects, &pSpriteObjects);

    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].containing_item.uItemID != ITEM_NULL && !(pSpriteObjects[i].uAttributes & SPRITE_MISSILE)) {
            pSpriteObjects[i].uType = static_cast<SPRITE_OBJECT_TYPE>(pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uSpriteID);
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    Deserialize(src.chests, &vChests);
    Deserialize(src.doors, &dst->pDoors);
    Deserialize(src.doorsData, &dst->ptr_0002B4_doors_ddata);

    for (uint i = 0, j = 0; i < dst->pDoors.size(); ++i) {
        BLVDoor *pDoor = &dst->pDoors[i];

        pDoor->pVertexIDs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumVertices;

        pDoor->pFaceIDs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumFaces;

        pDoor->pSectorIDs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumSectors;

        pDoor->pDeltaUs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumFaces;

        pDoor->pDeltaVs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumFaces;

        pDoor->pXOffsets = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumOffsets;

        pDoor->pYOffsets = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumOffsets;

        pDoor->pZOffsets = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumOffsets;

        assert(j <= dst->ptr_0002B4_doors_ddata.size());
    }

    for (size_t i = 0; i < dst->pDoors.size(); ++i) {
        BLVDoor *pDoor = &dst->pDoors[i];

        for (uint j = 0; j < pDoor->uNumFaces; ++j) {
            BLVFace *pFace = &dst->pFaces[pDoor->pFaceIDs[j]];
            BLVFaceExtra *pFaceExtra = &dst->pFaceExtras[pFace->uFaceExtraID];

            pDoor->pDeltaUs[j] = pFaceExtra->sTextureDeltaU;
            pDoor->pDeltaVs[j] = pFaceExtra->sTextureDeltaV;
        }
    }

    Deserialize(src.eventVariables, &mapEventVariables);
    Deserialize(src.locationTime, &dst->stru1);
}

void Serialize(const IndoorSave_MM7 &src, Blob *dst) {
    //assert(src.faceAttributes.size() == src.header.uNumFacesInBModels);
    //assert(src.decorationFlags.size() == src.header.uNumDecorations);

    BlobSerializer stream;
    stream.WriteRaw(&src.header);
    stream.WriteRaw(&src.visibleOutlines);
    stream.WriteSizedVector(src.faceAttributes);
    stream.WriteSizedVector(src.decorationFlags);
    stream.WriteVector(src.actors);
    stream.WriteVector(src.spriteObjects);
    stream.WriteVector(src.chests);
    stream.WriteSizedVector(src.doors);
    stream.WriteSizedVector(src.doorsData);
    stream.WriteRaw(&src.eventVariables);
    stream.WriteRaw(&src.locationTime);
    *dst = stream.Close();
}

void Deserialize(const Blob &src, IndoorSave_MM7 *dst, const IndoorLocation_MM7 &ctx, std::function<void()> progress) {
    BlobDeserializer stream(src);
    stream.ReadRaw(&dst->header);
    progress();
    stream.ReadRaw(&dst->visibleOutlines);
    progress();
    stream.ReadSizedVector(&dst->faceAttributes, ctx.faces.size());
    progress();
    stream.ReadSizedVector(&dst->decorationFlags, ctx.decorations.size());
    progress();
    stream.ReadVector(&dst->actors);
    progress();
    stream.ReadVector(&dst->spriteObjects);
    progress();
    stream.ReadVector(&dst->chests);
    progress();
    stream.ReadSizedVector(&dst->doors, ctx.doorCount);
    progress();
    stream.ReadSizedVector(&dst->doorsData, ctx.header.uDoors_ddata_Size / sizeof(int16_t));
    progress();
    stream.ReadRaw(&dst->eventVariables);
    progress();
    stream.ReadRaw(&dst->locationTime);
    progress();
}

