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

void Deserialize(InputStream &src, IndoorLocation_MM7 *dst, std::function<void()> progress) {
    progress();
    Deserialize(src, &dst->header);
    Deserialize(src, &dst->vertices);
    progress();
    progress();
    Deserialize(src, &dst->faces);
    Deserialize(src, presized(dst->header.uFaces_fdata_Size / sizeof(uint16_t), &dst->faceData));
    progress();
    Deserialize(src, presized(dst->faces.size(), &dst->faceTextures));
    progress();
    Deserialize(src, &dst->faceExtras);
    progress();
    Deserialize(src, presized(dst->faceExtras.size(), &dst->faceExtraTextures));
    progress();
    Deserialize(src, &dst->sectors);
    progress();
    Deserialize(src, presized(dst->header.uSector_rdata_Size / sizeof(uint16_t), &dst->sectorData));
    Deserialize(src, presized(dst->header.uSector_lrdata_Size / sizeof(uint16_t), &dst->sectorLightData));
    progress();
    progress();
    Deserialize(src, &dst->doorCount);
    progress();
    progress();
    Deserialize(src, &dst->decorations);
    Deserialize(src, presized(dst->decorations.size(), &dst->decorationNames));
    progress();
    Deserialize(src, &dst->lights);
    progress();
    progress();
    Deserialize(src, &dst->bspNodes);
    progress();
    progress();
    Deserialize(src, &dst->spawnPoints);
    progress();
    progress();
    Deserialize(src, &dst->mapOutlines);
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

void Serialize(const IndoorDelta_MM7 &src, OutputStream *dst) {
    Serialize(src.header, dst);
    Serialize(src.visibleOutlines, dst);
    Serialize(unsized(src.faceAttributes), dst);
    Serialize(unsized(src.decorationFlags), dst);
    Serialize(src.actors, dst);
    Serialize(src.spriteObjects, dst);
    Serialize(src.chests, dst);
    Serialize(unsized(src.doors), dst);
    Serialize(unsized(src.doorsData), dst);
    Serialize(src.eventVariables, dst);
    Serialize(src.locationTime, dst);
}

void Deserialize(InputStream &src, IndoorDelta_MM7 *dst, const IndoorLocation_MM7 &ctx, std::function<void()> progress) {
    Deserialize(src, &dst->header);
    Deserialize(src, &dst->visibleOutlines);
    Deserialize(src, presized(ctx.faces.size(), &dst->faceAttributes));
    progress();
    Deserialize(src, presized(ctx.decorations.size(), &dst->decorationFlags));
    progress();
    Deserialize(src, &dst->actors);
    progress();
    progress();
    Deserialize(src, &dst->spriteObjects);
    progress();
    Deserialize(src, &dst->chests);
    progress();
    progress();
    Deserialize(src, presized(ctx.doorCount, &dst->doors));
    Deserialize(src, presized(ctx.header.uDoors_ddata_Size / sizeof(int16_t), &dst->doorsData));
    progress();
    Deserialize(src, &dst->eventVariables);
    progress();
    Deserialize(src, &dst->locationTime);
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

void Deserialize(InputStream &src, OutdoorLocation_MM7 *dst, std::function<void()> progress) {
    Deserialize(src, &dst->name);
    Deserialize(src, &dst->fileName);
    Deserialize(src, &dst->desciption);
    Deserialize(src, &dst->skyTexture);
    Deserialize(src, &dst->groundTileset);
    Deserialize(src, &dst->tileTypes);
    progress();
    Deserialize(src, &dst->heightMap);
    Deserialize(src, &dst->tileMap);
    Deserialize(src, &dst->attributeMap);
    progress();
    Deserialize(src, &dst->normalCount);
    Deserialize(src, &dst->someOtherMap);
    Deserialize(src, &dst->normalMap);
    Deserialize(src, presized(dst->normalCount, &dst->normals));
    progress();
    Deserialize(src, &dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        Deserialize(src, presized(model.uNumVertices, &extra.vertices));
        Deserialize(src, presized(model.uNumFaces, &extra.faces));
        Deserialize(src, presized(model.uNumFaces, &extra.faceOrdering));
        Deserialize(src, presized(model.uNumNodes, &extra.bspNodes));
        Deserialize(src, presized(model.uNumFaces, &extra.faceTextures));
    }

    progress();
    Deserialize(src, &dst->decorations);
    progress();
    Deserialize(src, presized(dst->decorations.size(), &dst->decorationNames));
    progress();
    Deserialize(src, &dst->decorationPidList);
    progress();
    Deserialize(src, &dst->decorationMap);
    progress();
    progress();
    Deserialize(src, &dst->spawnPoints);
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

void Serialize(const OutdoorDelta_MM7 &src, OutputStream *dst) {
    Serialize(src.header, dst);
    Serialize(src.fullyRevealedCells, dst);
    Serialize(src.partiallyRevealedCells, dst);
    Serialize(unsized(src.faceAttributes), dst);
    Serialize(unsized(src.decorationFlags), dst);
    Serialize(src.actors, dst);
    Serialize(src.spriteObjects, dst);
    Serialize(src.chests, dst);
    Serialize(src.eventVariables, dst);
    Serialize(src.locationTime, dst);
}

void Deserialize(InputStream &src, OutdoorDelta_MM7 *dst, const OutdoorLocation_MM7 &ctx, std::function<void()> progress) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx.models)
        totalFaces += model.uNumFaces;

    Deserialize(src, &dst->header);
    Deserialize(src, &dst->fullyRevealedCells);
    Deserialize(src, &dst->partiallyRevealedCells);
    progress();
    Deserialize(src, presized(totalFaces, &dst->faceAttributes));
    progress();
    Deserialize(src, presized(ctx.decorations.size(), &dst->decorationFlags));
    progress();
    progress();
    Deserialize(src, &dst->actors);
    progress();
    progress();
    Deserialize(src, &dst->spriteObjects);
    progress();
    Deserialize(src, &dst->chests);
    progress();
    Deserialize(src, &dst->eventVariables);
    progress();
    Deserialize(src, &dst->locationTime);
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
    dst->Write("header.bin", toBlob(src.header));
    dst->Write("party.bin", toBlob(src.party));
    dst->Write("clock.bin", toBlob(src.eventTimer));
    dst->Write("overlay.bin", toBlob(src.overlays));
    dst->Write("npcdata.bin", toBlob(src.npcData));
    dst->Write("npcgroup.bin", toBlob(src.npcGroup));
}

void Deserialize(const LOD::File &src, SaveGame_MM7 *dst) {
    Deserialize(src.LoadRaw("header.bin"), &dst->header);
    Deserialize(src.LoadRaw("party.bin"), &dst->party);
    Deserialize(src.LoadRaw("clock.bin"), &dst->eventTimer);
    Deserialize(src.LoadRaw("overlay.bin"), &dst->overlays);
    Deserialize(src.LoadRaw("npcdata.bin"), &dst->npcData);
    Deserialize(src.LoadRaw("npcgroup.bin"), &dst->npcGroup);
}
