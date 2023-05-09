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

#include "CommonImages.h"

void deserialize(const IndoorLocation_MM7 &src, IndoorLocation *dst) {
    deserialize(src.vertices, &dst->pVertices);
    deserialize(src.faces, &dst->pFaces);
    deserialize(src.faceData, &dst->pLFaces);

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
        deserialize(src.faceTextures[i], &texName);
        pFace->SetTexture(texName);
    }

    deserialize(src.faceExtras, &dst->pFaceExtras);

    std::string textureName;
    for (uint i = 0; i < dst->pFaceExtras.size(); ++i) {
        deserialize(src.faceExtraTextures[i], &textureName);

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

    deserialize(src.sectors, &dst->pSectors);
    deserialize(src.sectorData, &dst->ptr_0002B0_sector_rdata);

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

    deserialize(src.sectorLightData, &dst->ptr_0002B8_sector_lrdata);

    for (uint i = 0, j = 0; i < dst->pSectors.size(); ++i) {
        BLVSector *pSector = &dst->pSectors[i];

        pSector->pLights = dst->ptr_0002B8_sector_lrdata.data() + j;
        j += pSector->uNumLights;

        assert(j <= dst->ptr_0002B8_sector_lrdata.size());
    }

    deserialize(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        deserialize(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    deserialize(src.lights, &dst->pLights);
    deserialize(src.bspNodes, &dst->pNodes);
    deserialize(src.spawnPoints, &dst->pSpawnPoints);
    deserialize(src.mapOutlines, &dst->pMapOutlines);
}

void deserialize(InputStream &src, IndoorLocation_MM7 *dst) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->vertices);
    deserialize(src, &dst->faces);
    deserialize(src, presized(dst->header.uFaces_fdata_Size / sizeof(uint16_t), &dst->faceData));
    deserialize(src, presized(dst->faces.size(), &dst->faceTextures));
    deserialize(src, &dst->faceExtras);
    deserialize(src, presized(dst->faceExtras.size(), &dst->faceExtraTextures));
    deserialize(src, &dst->sectors);
    deserialize(src, presized(dst->header.uSector_rdata_Size / sizeof(uint16_t), &dst->sectorData));
    deserialize(src, presized(dst->header.uSector_lrdata_Size / sizeof(uint16_t), &dst->sectorLightData));
    deserialize(src, &dst->doorCount);
    deserialize(src, &dst->decorations);
    deserialize(src, presized(dst->decorations.size(), &dst->decorationNames));
    deserialize(src, &dst->lights);
    deserialize(src, &dst->bspNodes);
    deserialize(src, &dst->spawnPoints);
    deserialize(src, &dst->mapOutlines);
}

void serialize(const IndoorLocation &src, IndoorDelta_MM7 *dst) {
    serialize(src.dlv, &dst->header.info);
    dst->header.totalFacesCount = src.pFaces.size();
    dst->header.bmodelCount = 0;
    dst->header.decorationCount = pLevelDecorations.size();

    serialize(src._visible_outlines, &dst->visibleOutlines);

    dst->faceAttributes.clear();
    for (const BLVFace &pFace : pIndoor->pFaces)
        dst->faceAttributes.push_back(std::to_underlying(pFace.uAttributes));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    serialize(pActors, &dst->actors);
    serialize(pSpriteObjects, &dst->spriteObjects);
    serialize(vChests, &dst->chests);
    serialize(src.pDoors, &dst->doors);
    serialize(src.ptr_0002B4_doors_ddata, &dst->doorsData);
    serialize(engine->_persistentVariables, &dst->eventVariables);
    serialize(src.stru1, &dst->locationTime);
}

void deserialize(const IndoorDelta_MM7 &src, IndoorLocation *dst) {
    deserialize(src.header.info, &dst->dlv); // XXX
    deserialize(src.visibleOutlines, &dst->_visible_outlines);

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

    deserialize(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    deserialize(src.spriteObjects, &pSpriteObjects);

    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].containing_item.uItemID != ITEM_NULL && !(pSpriteObjects[i].uAttributes & SPRITE_MISSILE)) {
            pSpriteObjects[i].uType = static_cast<SPRITE_OBJECT_TYPE>(pItemTable->pItems[pSpriteObjects[i].containing_item.uItemID].uSpriteID);
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    deserialize(src.chests, &vChests);
    deserialize(src.doors, &dst->pDoors);
    deserialize(src.doorsData, &dst->ptr_0002B4_doors_ddata);

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

    deserialize(src.eventVariables, &engine->_persistentVariables);
    deserialize(src.locationTime, &dst->stru1);
}

void serialize(const IndoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.visibleOutlines, dst);
    serialize(unsized(src.faceAttributes), dst);
    serialize(unsized(src.decorationFlags), dst);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(unsized(src.doors), dst);
    serialize(unsized(src.doorsData), dst);
    serialize(src.eventVariables, dst);
    serialize(src.locationTime, dst);
}

void deserialize(InputStream &src, IndoorDelta_MM7 *dst, const IndoorLocation_MM7 &ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, presized(ctx.faces.size(), &dst->faceAttributes));
    deserialize(src, presized(ctx.decorations.size(), &dst->decorationFlags));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, presized(ctx.doorCount, &dst->doors));
    deserialize(src, presized(ctx.header.uDoors_ddata_Size / sizeof(int16_t), &dst->doorsData));
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->locationTime);
}

void deserialize(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst) {
    const auto &[srcData, srcExtras] = src;

    // dst->index is set externally.
    deserialize(srcData.pModelName, &dst->pModelName);
    deserialize(srcData.pModelName2, &dst->pModelName2);
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
    deserialize(srcExtras.faces, &dst->pFaces);

    for (size_t i = 0; i < dst->pFaces.size(); i++)
        dst->pFaces[i].index = i;

    dst->pFacesOrdering = srcExtras.faceOrdering;

    deserialize(srcExtras.bspNodes, &dst->pNodes);

    std::string textureName;
    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        deserialize(srcExtras.faceTextures[i], &textureName);
        dst->pFaces[i].SetTexture(textureName);

        if (dst->pFaces[i].sCogTriggeredID) {
            if (dst->pFaces[i].HasEventHint())
                dst->pFaces[i].uAttributes |= FACE_HAS_EVENT;
            else
                dst->pFaces[i].uAttributes &= ~FACE_HAS_EVENT;
        }
    }
}

void deserialize(const OutdoorLocation_MM7 &src, OutdoorLocation *dst) {
    deserialize(src.name, &dst->level_filename);
    deserialize(src.fileName, &dst->location_filename);
    deserialize(src.desciption, &dst->location_file_description);
    deserialize(src.skyTexture, &dst->sky_texture_filename);
    // src.groundTileset is just dropped
    deserialize(src.tileTypes, &dst->pTileTypes);

    dst->LoadTileGroupIds();
    dst->LoadRoadTileset();

    deserialize(src.heightMap, &dst->pTerrain.pHeightmap);
    deserialize(src.tileMap, &dst->pTerrain.pTilemap);
    deserialize(src.attributeMap, &dst->pTerrain.pAttributemap);

    dst->pTerrain.FillDMap(0, 0, 128, 128);

    deserialize(src.someOtherMap, &pTerrainSomeOtherData);
    deserialize(src.normalMap, &pTerrainNormalIndices);
    deserialize(src.normals, &pTerrainNormals);

    dst->pBModels.clear();
    for (size_t i = 0; i < src.models.size(); i++) {
        BSPModel &dstModel = dst->pBModels.emplace_back();
        dstModel.index = i;
        deserialize(std::forward_as_tuple(src.models[i], src.modelExtras[i]), &dstModel);
    }

    deserialize(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        deserialize(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    deserialize(src.decorationPidList, &dst->pFaceIDLIST);
    deserialize(src.decorationMap, &dst->pOMAP);
    deserialize(src.spawnPoints, &dst->pSpawnPoints);
}

void deserialize(InputStream &src, OutdoorLocation_MM7 *dst) {
    deserialize(src, &dst->name);
    deserialize(src, &dst->fileName);
    deserialize(src, &dst->desciption);
    deserialize(src, &dst->skyTexture);
    deserialize(src, &dst->groundTileset);
    deserialize(src, &dst->tileTypes);
    deserialize(src, &dst->heightMap);
    deserialize(src, &dst->tileMap);
    deserialize(src, &dst->attributeMap);
    deserialize(src, &dst->normalCount);
    deserialize(src, &dst->someOtherMap);
    deserialize(src, &dst->normalMap);
    deserialize(src, presized(dst->normalCount, &dst->normals));
    deserialize(src, &dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        deserialize(src, presized(model.uNumVertices, &extra.vertices));
        deserialize(src, presized(model.uNumFaces, &extra.faces));
        deserialize(src, presized(model.uNumFaces, &extra.faceOrdering));
        deserialize(src, presized(model.uNumNodes, &extra.bspNodes));
        deserialize(src, presized(model.uNumFaces, &extra.faceTextures));
    }

    deserialize(src, &dst->decorations);
    deserialize(src, presized(dst->decorations.size(), &dst->decorationNames));
    deserialize(src, &dst->decorationPidList);
    deserialize(src, &dst->decorationMap);
    deserialize(src, &dst->spawnPoints);
}

void serialize(const OutdoorLocation &src, OutdoorDelta_MM7 *dst) {
    serialize(src.ddm, &dst->header.info);
    dst->header.totalFacesCount = 0;
    for (const BSPModel &model : src.pBModels)
        dst->header.totalFacesCount += model.pFaces.size();
    dst->header.bmodelCount = src.pBModels.size();
    dst->header.decorationCount = pLevelDecorations.size();

    serialize(src.uFullyRevealedCellOnMap, &dst->fullyRevealedCells);
    serialize(src.uPartiallyRevealedCellOnMap, &dst->partiallyRevealedCells);

    dst->faceAttributes.clear();
    for (const BSPModel &model : src.pBModels)
        for (const ODMFace &face : model.pFaces)
            dst->faceAttributes.push_back(std::to_underlying(face.uAttributes));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    serialize(pActors, &dst->actors);
    serialize(pSpriteObjects, &dst->spriteObjects);
    serialize(vChests, &dst->chests);
    serialize(engine->_persistentVariables, &dst->eventVariables);
    serialize(src.loc_time, &dst->locationTime);
}

void deserialize(const OutdoorDelta_MM7 &src, OutdoorLocation *dst) {
    deserialize(src.header.info, &dst->ddm);
    deserialize(src.fullyRevealedCells, &dst->uFullyRevealedCellOnMap);
    deserialize(src.partiallyRevealedCells, &dst->uPartiallyRevealedCellOnMap);

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

    deserialize(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    deserialize(src.spriteObjects, &pSpriteObjects);
    deserialize(src.chests, &vChests);
    deserialize(src.eventVariables, &engine->_persistentVariables);
    deserialize(src.locationTime, &dst->loc_time);
}

void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.fullyRevealedCells, dst);
    serialize(src.partiallyRevealedCells, dst);
    serialize(unsized(src.faceAttributes), dst);
    serialize(unsized(src.decorationFlags), dst);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.eventVariables, dst);
    serialize(src.locationTime, dst);
}

void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, const OutdoorLocation_MM7 &ctx) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx.models)
        totalFaces += model.uNumFaces;

    deserialize(src, &dst->header);
    deserialize(src, &dst->fullyRevealedCells);
    deserialize(src, &dst->partiallyRevealedCells);
    deserialize(src, presized(totalFaces, &dst->faceAttributes));
    deserialize(src, presized(ctx.decorations.size(), &dst->decorationFlags));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->locationTime);
}

void serialize(const SaveGameHeader &src, SaveGame_MM7 *dst) {
    serialize(src, &dst->header);
    serialize(*pParty, &dst->party);
    serialize(*pEventTimer, &dst->eventTimer);
    serialize(*pActiveOverlayList, &dst->overlays);
    serialize(pNPCStats->pNewNPCData, &dst->npcData);
    serialize(pNPCStats->pGroups_copy, &dst->npcGroup);
}

void deserialize(const SaveGame_MM7 &src, SaveGameHeader *dst) {
    deserialize(src.header, dst);
    deserialize(src.party, pParty);
    deserialize(src.eventTimer, pEventTimer);
    deserialize(src.overlays, pActiveOverlayList);
    deserialize(src.npcData, &pNPCStats->pNewNPCData);
    deserialize(src.npcGroup, &pNPCStats->pGroups_copy);
}

void serialize(const SaveGame_MM7 &src, LOD::WriteableFile *dst) {
    dst->Write("header.bin", toBlob(src.header));
    dst->Write("party.bin", toBlob(src.party));
    dst->Write("clock.bin", toBlob(src.eventTimer));
    dst->Write("overlay.bin", toBlob(src.overlays));
    dst->Write("npcdata.bin", toBlob(src.npcData));
    dst->Write("npcgroup.bin", toBlob(src.npcGroup));
}

void deserialize(const LOD::File &src, SaveGame_MM7 *dst) {
    deserialize(src.LoadRaw("header.bin"), &dst->header);
    deserialize(src.LoadRaw("party.bin"), &dst->party);
    deserialize(src.LoadRaw("clock.bin"), &dst->eventTimer);
    deserialize(src.LoadRaw("overlay.bin"), &dst->overlays);
    deserialize(src.LoadRaw("npcdata.bin"), &dst->npcData);
    deserialize(src.LoadRaw("npcgroup.bin"), &dst->npcGroup);
}
