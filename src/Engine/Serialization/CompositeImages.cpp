#include "CompositeImages.h"

#include <string>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Overlays.h"
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

    std::string textureName;
    for (uint i = 0; i < dst->pFaceExtras.size(); ++i) {
        Deserialize(src.faceExtraTextures[i], &textureName);

        if (textureName.empty())
            dst->pFaceExtras[i].uAdditionalBitmapID = -1;
        else
            dst->pFaceExtras[i].uAdditionalBitmapID = pBitmaps_LOD->LoadTexture(textureName);
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

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        Deserialize(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
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

void Serialize(const IndoorLocation &src, IndoorDelta_MM7 *dst) {
    Serialize(src.dlv, &dst->header);
    Serialize(src._visible_outlines, &dst->visibleOutlines);

    dst->faceAttributes.clear();
    for (const BLVFace &pFace : pIndoor->pFaces)
        dst->faceAttributes.push_back(std::to_underlying(pFace.uAttributes));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    Serialize(pActors, &dst->actors);
    Serialize(pSpriteObjects, &dst->spriteObjects);
    Serialize(vChests, &dst->chests);
    Serialize(src.pDoors, &dst->doors);
    Serialize(src.ptr_0002B4_doors_ddata, &dst->doorsData);
    Serialize(mapEventVariables, &dst->eventVariables);
    Serialize(src.stru1, &dst->locationTime);
}

void Deserialize(const IndoorDelta_MM7 &src, IndoorLocation *dst) {
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

void Serialize(const IndoorDelta_MM7 &src, Blob *dst) {
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
    *dst = stream.Reset();
}

void Deserialize(const Blob &src, IndoorDelta_MM7 *dst, const IndoorLocation_MM7 &ctx, std::function<void()> progress) {
    BlobDeserializer stream(src);
    stream.ReadRaw(&dst->header);
    stream.ReadRaw(&dst->visibleOutlines);
    stream.ReadSizedVector(&dst->faceAttributes, ctx.faces.size());
    progress();
    stream.ReadSizedVector(&dst->decorationFlags, ctx.decorations.size());
    progress();
    stream.ReadVector(&dst->actors);
    progress();
    progress();
    stream.ReadVector(&dst->spriteObjects);
    progress();
    stream.ReadVector(&dst->chests);
    progress();
    progress();
    stream.ReadSizedVector(&dst->doors, ctx.doorCount);
    stream.ReadSizedVector(&dst->doorsData, ctx.header.uDoors_ddata_Size / sizeof(int16_t));
    progress();
    stream.ReadRaw(&dst->eventVariables);
    progress();
    stream.ReadRaw(&dst->locationTime);
    progress();
}

void Deserialize(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst) {
    const auto &[srcData, srcExtras] = src;

    // dst->index is set externally.
    Deserialize(srcData.pModelName, &dst->pModelName);
    Deserialize(srcData.pModelName2, &dst->pModelName2);
    dst->field_40 = srcData.field_40;
    dst->sCenterX = srcData.sCenterX;
    dst->sCenterY = srcData.sCenterY;
    dst->vPosition = srcData.vPosition;
    dst->pBoundingBox.x1 = srcData.sMinX;
    dst->pBoundingBox.y1 = srcData.sMinY;
    dst->pBoundingBox.z1 = srcData.sMinZ;
    dst->pBoundingBox.x2 = srcData.sMaxX;
    dst->pBoundingBox.y2 = srcData.sMaxY;
    dst->pBoundingBox.z2 = srcData.sMaxZ;
    dst->sSomeOtherMinX = srcData.sSomeOtherMinX;
    dst->sSomeOtherMinY = srcData.sSomeOtherMinY;
    dst->sSomeOtherMinZ = srcData.sSomeOtherMinZ;
    dst->sSomeOtherMaxX = srcData.sSomeOtherMaxX;
    dst->sSomeOtherMaxY = srcData.sSomeOtherMaxY;
    dst->sSomeOtherMaxZ = srcData.sSomeOtherMaxZ;
    dst->vBoundingCenter = srcData.vBoundingCenter;
    dst->sBoundingRadius = srcData.sBoundingRadius;

    dst->pVertices = srcExtras.vertices;
    Deserialize(srcExtras.faces, &dst->pFaces);

    for (size_t i = 0; i < dst->pFaces.size(); i++)
        dst->pFaces[i].index = i;

    dst->pFacesOrdering = srcExtras.faceOrdering;

    Deserialize(srcExtras.bspNodes, &dst->pNodes);

    std::string textureName;
    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        Deserialize(srcExtras.faceTextures[i], &textureName);
        dst->pFaces[i].SetTexture(textureName);

        if (dst->pFaces[i].sCogTriggeredID) {
            if (dst->pFaces[i].HasEventHint())
                dst->pFaces[i].uAttributes |= FACE_HAS_EVENT;
            else
                dst->pFaces[i].uAttributes &= ~FACE_HAS_EVENT;
        }
    }
}

void Deserialize(const OutdoorLocation_MM7 &src, OutdoorLocation *dst) {
    Deserialize(src.name, &dst->level_filename);
    Deserialize(src.fileName, &dst->location_filename);
    Deserialize(src.desciption, &dst->location_file_description);
    Deserialize(src.skyTexture, &dst->sky_texture_filename);
    // src.groundTileset is just dropped
    Deserialize(src.tileTypes, &dst->pTileTypes);

    dst->LoadTileGroupIds();
    dst->LoadRoadTileset();

    Deserialize(src.heightMap, &dst->pTerrain.pHeightmap);
    Deserialize(src.tileMap, &dst->pTerrain.pTilemap);
    Deserialize(src.attributeMap, &dst->pTerrain.pAttributemap);

    dst->pTerrain.FillDMap(0, 0, 128, 128);

    Deserialize(src.someOtherMap, &pTerrainSomeOtherData);
    Deserialize(src.normalMap, &pTerrainNormalIndices);
    Deserialize(src.normals, &pTerrainNormals);

    dst->pBModels.clear();
    for (size_t i = 0; i < src.models.size(); i++) {
        BSPModel &dstModel = dst->pBModels.emplace_back();
        dstModel.index = i;
        Deserialize(std::forward_as_tuple(src.models[i], src.modelExtras[i]), &dstModel);
    }

    Deserialize(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        Deserialize(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    Deserialize(src.decorationPidList, &dst->pFaceIDLIST);
    Deserialize(src.decorationMap, &dst->pOMAP);
    Deserialize(src.spawnPoints, &dst->pSpawnPoints);
}

void Deserialize(const Blob &src, OutdoorLocation_MM7 *dst, std::function<void()> progress) {
    BlobDeserializer stream(src);
    stream.ReadRaw(&dst->name);
    stream.ReadRaw(&dst->fileName);
    stream.ReadRaw(&dst->desciption);
    stream.ReadRaw(&dst->skyTexture);
    stream.ReadRaw(&dst->groundTileset);
    stream.ReadRaw(&dst->tileTypes);
    progress();
    stream.ReadRaw(&dst->heightMap);
    stream.ReadRaw(&dst->tileMap);
    stream.ReadRaw(&dst->attributeMap);
    progress();
    stream.ReadRaw(&dst->normalCount);
    stream.ReadRaw(&dst->someOtherMap);
    stream.ReadRaw(&dst->normalMap);
    stream.ReadSizedVector(&dst->normals, dst->normalCount);
    progress();
    stream.ReadVector(&dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        stream.ReadSizedVector(&extra.vertices, model.uNumVertices);
        stream.ReadSizedVector(&extra.faces, model.uNumFaces);
        stream.ReadSizedVector(&extra.faceOrdering, model.uNumFaces);
        stream.ReadSizedVector(&extra.bspNodes, model.uNumNodes);
        stream.ReadSizedVector(&extra.faceTextures, model.uNumFaces);
    }

    progress();
    stream.ReadVector(&dst->decorations);
    progress();
    stream.ReadSizedVector(&dst->decorationNames, dst->decorations.size());
    progress();
    stream.ReadVector(&dst->decorationPidList);
    progress();
    stream.ReadRaw(&dst->decorationMap);
    progress();
    progress();
    stream.ReadVector<SpawnPoint_MM7>(&dst->spawnPoints);
    progress();
}

void Serialize(const OutdoorLocation &src, OutdoorDelta_MM7 *dst) {
    Serialize(src.ddm, &dst->header);
    Serialize(src.uFullyRevealedCellOnMap, &dst->fullyRevealedCells);
    Serialize(src.uPartiallyRevealedCellOnMap, &dst->partiallyRevealedCells);

    dst->faceAttributes.clear();
    for (const BSPModel &model : src.pBModels)
        for (const ODMFace &face : model.pFaces)
            dst->faceAttributes.push_back(std::to_underlying(face.uAttributes));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    Serialize(pActors, &dst->actors);
    Serialize(pSpriteObjects, &dst->spriteObjects);
    Serialize(vChests, &dst->chests);
    Serialize(mapEventVariables, &dst->eventVariables);
    Serialize(src.loc_time, &dst->locationTime);
}

void Deserialize(const OutdoorDelta_MM7 &src, OutdoorLocation *dst) {
    Deserialize(src.header, &dst->ddm);
    Deserialize(src.fullyRevealedCells, &dst->uFullyRevealedCellOnMap);
    Deserialize(src.partiallyRevealedCells, &dst->uPartiallyRevealedCellOnMap);

    size_t attributeIndex = 0;
    for (BSPModel &model : dst->pBModels) {
        for (ODMFace &face : model.pFaces)
            face.uAttributes = FaceAttributes(src.faceAttributes[attributeIndex++]);

        for (ODMFace &face : model.pFaces) {
            if (face.sCogTriggeredID) {
                if (face.HasEventHint()) {
                    face.uAttributes |= FACE_HAS_EVENT;
                } else {
                    face.uAttributes &= ~FACE_HAS_EVENT;
                }
            }
        }

        // TODO(captainurist): this actually belongs to level loading code, not save loading
        // calculate bounding sphere for model
        Vec3f topLeft = Vec3f(model.pBoundingBox.x1, model.pBoundingBox.y1, model.pBoundingBox.z1);
        Vec3f bottomRight = Vec3f(model.pBoundingBox.x2, model.pBoundingBox.y2, model.pBoundingBox.z2);
        model.vBoundingCenter = ((topLeft + bottomRight) / 2.0f).toInt();
        model.sBoundingRadius = (topLeft - model.vBoundingCenter.toFloat()).length();
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    Deserialize(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    Deserialize(src.spriteObjects, &pSpriteObjects);
    Deserialize(src.chests, &vChests);
    Deserialize(src.eventVariables, &mapEventVariables);
    Deserialize(src.locationTime, &dst->loc_time);
}

void Serialize(const OutdoorDelta_MM7 &src, Blob *dst) {
    BlobSerializer stream;
    stream.WriteRaw(&src.header);
    stream.WriteRaw(&src.fullyRevealedCells);
    stream.WriteRaw(&src.partiallyRevealedCells);
    stream.WriteSizedVector(src.faceAttributes);
    stream.WriteSizedVector(src.decorationFlags);
    stream.WriteVector(src.actors);
    stream.WriteVector(src.spriteObjects);
    stream.WriteVector(src.chests);
    stream.WriteRaw(&src.eventVariables);
    stream.WriteRaw(&src.locationTime);
    *dst = stream.Reset();
}

void Deserialize(const Blob &src, OutdoorDelta_MM7 *dst, const OutdoorLocation_MM7 &ctx, std::function<void()> progress) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx.models)
        totalFaces += model.uNumFaces;

    BlobDeserializer stream(src);
    stream.ReadRaw(&dst->header);
    stream.ReadRaw(&dst->fullyRevealedCells);
    stream.ReadRaw(&dst->partiallyRevealedCells);
    progress();
    stream.ReadSizedVector(&dst->faceAttributes, totalFaces);
    progress();
    stream.ReadSizedVector(&dst->decorationFlags, ctx.decorations.size());
    progress();
    progress();
    stream.ReadVector(&dst->actors);
    progress();
    progress();
    stream.ReadVector(&dst->spriteObjects);
    progress();
    stream.ReadVector(&dst->chests);
    progress();
    stream.ReadRaw(&dst->eventVariables);
    progress();
    stream.ReadRaw(&dst->locationTime);
}

void Serialize(const SaveGameHeader &src, SaveGame_MM7 *dst) {
    Serialize(src, &dst->header);
    Serialize(*pParty, &dst->party);
    Serialize(*pEventTimer, &dst->eventTimer);
    Serialize(*pOtherOverlayList, &dst->overlays);
    Serialize(pNPCStats->pNewNPCData, &dst->npcData);
    Serialize(pNPCStats->pGroups_copy, &dst->npcGroup);
}

void Deserialize(const SaveGame_MM7 &src, SaveGameHeader *dst) {
    Deserialize(src.header, dst);
    Deserialize(src.party, pParty);
    Deserialize(src.eventTimer, pEventTimer);
    Deserialize(src.overlays, pOtherOverlayList);
    Deserialize(src.npcData, &pNPCStats->pNewNPCData);
    Deserialize(src.npcGroup, &pNPCStats->pGroups_copy);
}

void Serialize(const SaveGame_MM7 &src, LOD::WriteableFile *dst) {
    BlobSerializer stream;
    stream.WriteRaw(&src.header);
    dst->Write("header.bin", stream.Reset());
    stream.WriteRaw(&src.party);
    dst->Write("party.bin", stream.Reset());
    stream.WriteRaw(&src.eventTimer);
    dst->Write("clock.bin", stream.Reset());
    stream.WriteRaw(&src.overlays);
    dst->Write("overlay.bin", stream.Reset());
    stream.WriteRaw(&src.npcData);
    dst->Write("npcdata.bin", stream.Reset());
    stream.WriteRaw(&src.npcGroup);
    dst->Write("npcgroup.bin", stream.Reset());
}

void Deserialize(const LOD::File &src, SaveGame_MM7 *dst) {
    BlobDeserializer stream;
    stream.Reset(src.LoadRaw("header.bin"));
    stream.ReadRaw(&dst->header);
    stream.Reset(src.LoadRaw("party.bin"));
    stream.ReadRaw(&dst->party);
    stream.Reset(src.LoadRaw("clock.bin"));
    stream.ReadRaw(&dst->eventTimer);
    stream.Reset(src.LoadRaw("overlay.bin"));
    stream.ReadRaw(&dst->overlays);
    stream.Reset(src.LoadRaw("npcdata.bin"));
    stream.ReadRaw(&dst->npcData);
    stream.Reset(src.LoadRaw("npcgroup.bin"));
    stream.ReadRaw(&dst->npcGroup);
}
