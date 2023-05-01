#include "CompositeImages.h"

#include <string>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/DecorationList.h"
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
}

void Serialize(const IndoorSave_MM7 &src, Blob *dst) {
    assert(src.faceAttributes.size() == src.header.uNumFacesInBModels);
    assert(src.decorationFlags.size() == src.header.uNumDecorations);

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

void Deserialize(const Blob &src, IndoorSave_MM7 *dst, size_t doorCount, size_t doorDataCount, std::function<void()> progress) {
    BlobDeserializer stream(src);
    stream.ReadRaw(&dst->header);
    progress();
    stream.ReadRaw(&dst->visibleOutlines);
    progress();
    stream.ReadSizedVector(&dst->faceAttributes, dst->header.uNumFacesInBModels);
    progress();
    stream.ReadSizedVector(&dst->decorationFlags, dst->header.uNumDecorations);
    progress();
    stream.ReadVector(&dst->actors);
    progress();
    stream.ReadVector(&dst->spriteObjects);
    progress();
    stream.ReadVector(&dst->chests);
    progress();
    stream.ReadSizedVector(&dst->doors, doorCount);
    progress();
    stream.ReadSizedVector(&dst->doorsData, doorDataCount);
    progress();
    stream.ReadRaw(&dst->eventVariables);
    progress();
    stream.ReadRaw(&dst->locationTime);
    progress();
}

