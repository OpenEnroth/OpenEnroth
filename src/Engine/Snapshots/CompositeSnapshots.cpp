#include "CompositeSnapshots.h"

#include <string>
#include <algorithm>
#include <tuple>
#include <utility>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Engine/Tables/TileTable.h"

#include "GUI/GUIFont.h"

#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Lod/LodWriter.h"
#include "Library/Lod/LodReader.h"
#include "Library/Lod/LodEnums.h"
#include "Library/Image/Pcx.h"

#include "Utility/Streams/BlobOutputStream.h"

#include "Engine/Graphics/Image.h"

void reconstruct(const IndoorLocation_MM7 &src, IndoorLocation *dst) {
    reconstruct(src.vertices, &dst->vertices);
    reconstruct(src.faces, &dst->faces);

    reconstruct(src.faceData, &dst->faceData);

    for (size_t i = 0, j = 0; i < dst->faces.size(); ++i) {
        BLVFace *pFace = &dst->faces[i];

        pFace->vertexIds = dst->faceData.data() + j;
        j += pFace->numVertices + 1;

        // Skipping pXInterceptDisplacements.
        j += pFace->numVertices + 1;

        // Skipping pYInterceptDisplacements.
        j += pFace->numVertices + 1;

        // Skipping pZInterceptDisplacements.
        j += pFace->numVertices + 1;

        pFace->textureUs = dst->faceData.data() + j;
        j += pFace->numVertices + 1;

        pFace->textureVs = dst->faceData.data() + j;
        j += pFace->numVertices + 1;

        assert(j <= dst->faceData.size());
    }

    // Face plane normals have come from fixed point values - recalculate them.
    for (auto& face : dst->faces) {
        if (face.numVertices < 3) continue;
        Vec3f dir1 = (dst->vertices[face.vertexIds[1]] - dst->vertices[face.vertexIds[0]]);
        int i = 2;
        // dir1 can be a 0 vec when first edge is degenerate - skip forwards
        while (dir1.length() < 1e-6f && i < face.numVertices) {
            dir1 = (dst->vertices[face.vertexIds[i]] - dst->vertices[face.vertexIds[i-1]]);
            i++;
        }

        Vec3f dir2, recalcNorm;
        for (; i < face.numVertices; i++) {
            dir2 = (dst->vertices[face.vertexIds[i]] - dst->vertices[face.vertexIds[0]]);
            if (recalcNorm = cross(dir1, dir2); recalcNorm.length() > 1e-6f) {
                recalcNorm /= recalcNorm.length();
                // Check that our new normal is pointing in the same direction as the original
                constexpr float tolerance = 0.95f; // TODO(pskelton): may need tuning
                if (dot(recalcNorm, face.facePlane.normal) > tolerance)
                    break;
            }
        }

        if (i == face.numVertices) {
            // If we didn't find a non-parallel edge, lets just round what were given.
            // TODO(pskelton):  This shouldnt ever happen - test and drop
            face.facePlane.normal /= face.facePlane.normal.length();
        } else {
            face.facePlane.normal = recalcNorm;
        }
        face.facePlane.dist = -dot(face.facePlane.normal, dst->vertices[face.vertexIds[0]]);
        face.zCalc.init(face.facePlane);
    }

    for (size_t i = 0; i < dst->faces.size(); ++i) {
        BLVFace *pFace = &dst->faces[i];

        std::string texName;
        reconstruct(src.faceTextures[i], &texName);
        pFace->SetTexture(texName);
    }

    reconstruct(src.faceExtras, &dst->faceExtras);

    std::string textureName;
    for (unsigned i = 0; i < dst->faceExtras.size(); ++i) {
        reconstruct(src.faceExtraTextures[i], &textureName);

        if (textureName.empty())
            dst->faceExtras[i].additionalBitmapId = -1;
        else
            dst->faceExtras[i].additionalBitmapId = -1; //pBitmaps_LOD->loadTexture(textureName); // TODO(captainurist): unused for some reason.
    }

    for (size_t i = 0; i < dst->faces.size(); ++i) {
        BLVFace *pFace = &dst->faces[i];
        BLVFaceExtra *pFaceExtra = &dst->faceExtras[pFace->faceExtraId];

        if (pFaceExtra->eventId) {
            if (pFaceExtra->HasEventHint())
                pFace->attributes |= FACE_HAS_EVENT;
            else
                pFace->attributes &= ~FACE_HAS_EVENT;
        }
    }

    reconstruct(src.sectors, &dst->sectors);
    reconstruct(src.sectorData, &dst->sectorData);

    for (size_t i = 0, j = 0; i < dst->sectors.size(); ++i) {
        BLVSector *dstSector = &dst->sectors[i];
        const BLVSector_MM7 &srcSector = src.sectors[i];

        dstSector->floorIds = std::span(dst->sectorData.data() + j, srcSector.numFloors);
        j += srcSector.numFloors;

        dstSector->wallIds = std::span(dst->sectorData.data() + j, srcSector.numWalls);
        j += srcSector.numWalls;

        dstSector->ceilingIds = std::span(dst->sectorData.data() + j, srcSector.numCeilings);
        j += srcSector.numCeilings;

        j += srcSector.numFluids; // Fluids not used in OE, skip.

        dstSector->portalIds = std::span(dst->sectorData.data() + j, srcSector.numPortals);
        j += srcSector.numPortals;

        dstSector->faceIds = std::span(dst->sectorData.data() + j, srcSector.numFaces);
        dstSector->nonBspFaceIds = dstSector->faceIds.subspan(0, srcSector.numNonBspFaces);
        j += srcSector.numFaces;

        j += srcSector.numCogs; // Cogs not used in OE, skip.

        dstSector->decorationIds = std::span(dst->sectorData.data() + j, srcSector.numDecorations);
        j += srcSector.numDecorations;

        j += srcSector.numMarkers; // Markers not used in OE, skip.

        assert(j <= dst->sectorData.size()); // TODO(captainurist): exception, not an assertion?
    }

    reconstruct(src.sectorLightData, &dst->sectorLightData);

    for (size_t i = 0, j = 0; i < dst->sectors.size(); ++i) {
        BLVSector *dstSector = &dst->sectors[i];
        const BLVSector_MM7 &srcSector = src.sectors[i];

        dstSector->lightIds = std::span(dst->sectorLightData.data() + j, srcSector.numLights);
        j += srcSector.numLights;

        assert(j <= dst->sectorLightData.size()); // TODO(captainurist): exception, not an assertion?
    }

    reconstruct(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        reconstruct(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    reconstruct(src.lights, &dst->lights);
    reconstruct(src.bspNodes, &dst->nodes);
    reconstruct(src.spawnPoints, &dst->pSpawnPoints);
    reconstruct(src.mapOutlines, &dst->mapOutlines);
}

void deserialize(InputStream &src, IndoorLocation_MM7 *dst) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->vertices);
    deserialize(src, &dst->faces);
    deserialize(src, &dst->faceData, tags::presized(dst->header.faceDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->faceTextures, tags::presized(dst->faces.size()));
    deserialize(src, &dst->faceExtras);
    deserialize(src, &dst->faceExtraTextures, tags::presized(dst->faceExtras.size()));
    deserialize(src, &dst->sectors);
    deserialize(src, &dst->sectorData, tags::presized(dst->header.sectorDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->sectorLightData, tags::presized(dst->header.sectorLightDataSizeBytes / sizeof(uint16_t)));
    deserialize(src, &dst->doorCount);
    deserialize(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    deserialize(src, &dst->lights);
    deserialize(src, &dst->bspNodes);
    deserialize(src, &dst->spawnPoints);
    deserialize(src, &dst->mapOutlines);
}

void snapshot(const IndoorLocation &src, IndoorDelta_MM7 *dst) {
    snapshot(src.dlv, &dst->header.info);
    dst->header.totalFacesCount = src.faces.size();
    dst->header.bmodelCount = 0;
    dst->header.decorationCount = pLevelDecorations.size();

    snapshot(src._visible_outlines, &dst->visibleOutlines);

    // Symmetric to what's happening in reconstruct - not all of the attributes need to be saved in a delta.
    dst->faceAttributes.clear();
    for (const BLVFace &pFace : pIndoor->faces)
        dst->faceAttributes.push_back(std::to_underlying(pFace.attributes & ~(FACE_HAS_EVENT | FACE_ANIMATED)));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    snapshot(pActors, &dst->actors);
    snapshot(pSpriteObjects, &dst->spriteObjects);
    snapshot(vChests, &dst->chests);
    snapshot(src.doors, &dst->doors);
    snapshot(src.doorsData, &dst->doorsData);
    snapshot(engine->_persistentVariables, &dst->eventVariables);
    snapshot(src.stru1, &dst->locationTime);
}

void reconstruct(const IndoorDelta_MM7 &src, IndoorLocation *dst) {
    reconstruct(src.header.info, &dst->dlv); // XXX
    reconstruct(src.visibleOutlines, &dst->_visible_outlines);

    for (size_t i = 0; i < dst->mapOutlines.size(); ++i) {
        BLVMapOutline *pVertex = &dst->mapOutlines[i];
        if ((uint8_t)(1 << (7 - i % 8)) & dst->_visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    // Not all of the attributes need to be restored.
    size_t attributeIndex = 0;
    for (BLVFace &face : dst->faces) {
        face.attributes &= FACE_ANIMATED | FACE_HAS_EVENT;
        face.attributes |= FaceAttributes(src.faceAttributes[attributeIndex++]) & ~(FACE_HAS_EVENT | FACE_ANIMATED);
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    reconstruct(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    reconstruct(src.spriteObjects, &pSpriteObjects);

    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].containing_item.itemId != ITEM_NULL && !(pSpriteObjects[i].uAttributes & SPRITE_MISSILE)) {
            pSpriteObjects[i].uType = static_cast<SpriteId>(pItemTable->items[pSpriteObjects[i].containing_item.itemId].spriteId);
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    vChests.resize(src.chests.size());
    for (size_t i = 0; i < src.chests.size(); ++i)
        reconstruct(src.chests[i], &vChests[i], tags::context<int>(i));

    reconstruct(src.doors, &dst->doors);
    reconstruct(src.doorsData, &dst->doorsData);

    for (unsigned i = 0, j = 0; i < dst->doors.size(); ++i) {
        BLVDoor *pDoor = &dst->doors[i];

        pDoor->pVertexIDs = dst->doorsData.data() + j;
        j += pDoor->numVertices;

        pDoor->pFaceIDs = dst->doorsData.data() + j;
        j += pDoor->numFaces;

        pDoor->pSectorIDs = dst->doorsData.data() + j;
        j += pDoor->numSectors;

        pDoor->pDeltaUs = dst->doorsData.data() + j;
        j += pDoor->numFaces;

        pDoor->pDeltaVs = dst->doorsData.data() + j;
        j += pDoor->numFaces;

        pDoor->pXOffsets = dst->doorsData.data() + j;
        j += pDoor->numOffsets;

        pDoor->pYOffsets = dst->doorsData.data() + j;
        j += pDoor->numOffsets;

        pDoor->pZOffsets = dst->doorsData.data() + j;
        j += pDoor->numOffsets;

        assert(j <= dst->doorsData.size());
    }

    for (size_t i = 0; i < dst->doors.size(); ++i) {
        BLVDoor *pDoor = &dst->doors[i];

        for (unsigned j = 0; j < pDoor->numFaces; ++j) {
            BLVFace *pFace = &dst->faces[pDoor->pFaceIDs[j]];
            BLVFaceExtra *pFaceExtra = &dst->faceExtras[pFace->faceExtraId];

            pDoor->pDeltaUs[j] = pFaceExtra->textureDeltaU;
            pDoor->pDeltaVs[j] = pFaceExtra->textureDeltaV;
        }
    }

    reconstruct(src.eventVariables, &engine->_persistentVariables);
    reconstruct(src.locationTime, &dst->stru1);
}

void serialize(const IndoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.visibleOutlines, dst);
    serialize(src.faceAttributes, dst, tags::unsized);
    serialize(src.decorationFlags, dst, tags::unsized);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.doors, dst, tags::unsized);
    serialize(src.doorsData, dst, tags::unsized);
    serialize(src.eventVariables, dst);
    serialize(src.locationTime, dst);
}

void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size()));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->doors, tags::presized(ctx->doorCount));
    deserialize(src, &dst->doorsData, tags::presized(ctx->header.doorsDataSizeBytes / sizeof(int16_t)));
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->locationTime);
}

void reconstruct(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst) {
    const auto &[srcData, srcExtras] = src;

    // dst->index is set externally.
    dst->field_40 = srcData.field_40;
    dst->position = srcData.position.toFloat();
    dst->boundingBox.x1 = srcData.minX;
    dst->boundingBox.y1 = srcData.minY;
    dst->boundingBox.z1 = srcData.minZ;
    dst->boundingBox.x2 = srcData.maxX;
    dst->boundingBox.y2 = srcData.maxY;
    dst->boundingBox.z2 = srcData.maxZ;
    dst->boundingCenter = srcData.boundingCenter.toFloat();
    dst->boundingRadius = srcData.boundingRadius;

    reconstruct(srcExtras.vertices, &dst->vertices);
    reconstruct(srcExtras.faces, &dst->faces);

    // Drop duplicate consecutive vertices in faces. This does happen in MM7 data.
    for (ODMFace &face : dst->faces) {
        int writeIdx = 0;
        for (int readIdx = 0; readIdx < face.numVertices; readIdx++) {
            if (face.vertexIds[readIdx] != face.vertexIds[(readIdx + 1) % face.numVertices]) {
                face.vertexIds[writeIdx] = face.vertexIds[readIdx];
                face.textureUs[writeIdx] = face.textureUs[readIdx];
                face.textureVs[writeIdx] = face.textureVs[readIdx];
                writeIdx++;
            }
        }
        face.numVertices = writeIdx;
    }

    // TODO(pskelton): This code is common to ODM/BLV faces
    // Face plane normals have come from fixed point values - recalculate them.
    for (auto& face : dst->faces) {
        if (face.numVertices < 3) continue;
        Vec3f dir1 = (dst->vertices[face.vertexIds[1]] - dst->vertices[face.vertexIds[0]]);
        Vec3f dir2, norm;
        int i = 2;
        for (; i < face.numVertices; i++) {
            dir2 = (dst->vertices[face.vertexIds[i]] - dst->vertices[face.vertexIds[0]]);
            if (norm = cross(dir1, dir2); norm.length() > 1e-6f) {
                break; // Found a non-parallel edge.
            }
        }

        if (i == face.numVertices) {
            // If we didn't find a non-parallel edge, lets just round what were given.
            // TODO(pskelton):  This shouldnt ever happen - test and drop
            face.facePlane.normal /= face.facePlane.normal.length();
        } else {
            face.facePlane.normal = norm / norm.length();
        }
        face.facePlane.dist = -dot(face.facePlane.normal, dst->vertices[face.vertexIds[0]]);
        face.zCalc.init(face.facePlane);
    }

    for (size_t i = 0; i < dst->faces.size(); i++)
        dst->faces[i].index = i;

    reconstruct(srcExtras.bspNodes, &dst->nodes);

    std::string textureName;
    for (size_t i = 0; i < dst->faces.size(); ++i) {
        reconstruct(srcExtras.faceTextures[i], &textureName);
        dst->faces[i].SetTexture(textureName);

        if (dst->faces[i].eventId) {
            if (dst->faces[i].HasEventHint())
                dst->faces[i].attributes |= FACE_HAS_EVENT;
            else
                dst->faces[i].attributes &= ~FACE_HAS_EVENT;
        }
    }
}

static int mapToGlobalTileId(const std::array<int, 4> &baseIds, int localTileId) {
    // Tiles in tilemap:
    // [0..90) are mapped as-is, but seem to be mostly invalid. Only global tile ids [1..12] are valid (all are dirt),
    //         the rest are "pending", effectively invalid.
    // [90..126) map to tileset #1.
    // [126..162) map to tileset #2.
    // [162..198) map to tileset #3.
    // [198..234) map to tileset #4 (road).
    // [234..255) are invalid.
    if (localTileId < 90)
        return localTileId;

    if (localTileId >= 234)
        return 0;

    int tilesetIndex = (localTileId - 90) / 36;
    int tilesetOffset = (localTileId - 90) % 36;
    return baseIds[tilesetIndex] + tilesetOffset;
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst) {
    std::array<int, 4> baseTileIds;
    for (int i = 0; i < 4; i++) {
        reconstruct(src.tileTypes[i].tileset, &dst->_tilesets[i]);
        baseTileIds[i] = pTileTable->tileId(dst->_tilesets[i], isRoad(dst->_tilesets[i]) ? TILE_VARIANT_ROAD_N_S_E_W : TILE_VARIANT_BASE1);
    }

    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++)
            dst->_heightMap[y][x] = src.heightMap[y * 128 + x];

    for (int y = 0; y < 127; y++)
        for (int x = 0; x < 127; x++)
            dst->_originalTileMap[y][x] = mapToGlobalTileId(baseTileIds, src.tileMap[y * 128 + x]);

    dst->recalculateNormals();
    dst->recalculateTransitions(&dst->_tileMap);

    dst->_tileMap = Image<int16_t>::copy(dst->_originalTileMap);
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorLocation *dst) {
    reconstruct(src.name, &dst->level_filename);
    reconstruct(src.fileName, &dst->location_filename);
    reconstruct(src.desciption, &dst->location_file_description);
    reconstruct(src.skyTexture, &dst->sky_texture_filename);
    reconstruct(src, &dst->pTerrain);

    dst->pBModels.clear();
    for (size_t i = 0; i < src.models.size(); i++) {
        BSPModel &model = dst->pBModels.emplace_back();
        model.index = i;
        reconstruct(std::forward_as_tuple(src.models[i], src.modelExtras[i]), &model);

        // Recalculate bounding spheres, the ones stored in data files are borked.
        model.boundingCenter = model.boundingBox.center().toFloat();
        model.boundingRadius = model.boundingBox.size().toFloat().length() / 2.0f;
    }

    reconstruct(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        reconstruct(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    reconstruct(src.decorationPidList, &dst->pFaceIDLIST);
    reconstruct(src.decorationMap, &dst->pOMAP);
    reconstruct(src.spawnPoints, &dst->pSpawnPoints);
}

void deserialize(InputStream &src, OutdoorLocation_MM7 *dst) {
    deserialize(src, &dst->name);
    deserialize(src, &dst->fileName);
    deserialize(src, &dst->desciption);
    deserialize(src, &dst->skyTexture);
    deserialize(src, &dst->groundTilesetUnused);
    deserialize(src, &dst->tileTypes);
    deserialize(src, &dst->heightMap);
    deserialize(src, &dst->tileMap);
    deserialize(src, &dst->attributeMap);
    deserialize(src, &dst->normalCount);
    deserialize(src, &dst->someOtherMap);
    deserialize(src, &dst->normalMap);
    deserialize(src, &dst->normals, tags::presized(dst->normalCount));
    deserialize(src, &dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        deserialize(src, &extra.vertices, tags::presized(model.numVertices));
        deserialize(src, &extra.faces, tags::presized(model.numFaces));
        deserialize(src, &extra.facesOrdering, tags::presized(model.numFaces));
        deserialize(src, &extra.bspNodes, tags::presized(model.numNodes));
        deserialize(src, &extra.faceTextures, tags::presized(model.numFaces));
    }

    deserialize(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    deserialize(src, &dst->decorationPidList);
    deserialize(src, &dst->decorationMap);
    deserialize(src, &dst->spawnPoints);
}

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM7 *dst) {
    snapshot(src.ddm, &dst->header.info);
    dst->header.totalFacesCount = 0;
    for (const BSPModel &model : src.pBModels)
        dst->header.totalFacesCount += model.faces.size();
    dst->header.bmodelCount = src.pBModels.size();
    dst->header.decorationCount = pLevelDecorations.size();

    snapshot(src.uFullyRevealedCellOnMap, &dst->fullyRevealedCells);
    snapshot(src.uPartiallyRevealedCellOnMap, &dst->partiallyRevealedCells);

    // Symmetric to what's happening in reconstruct - no all attributes need to be saved in a delta.
    dst->faceAttributes.clear();
    for (const BSPModel &model : src.pBModels)
        for (const ODMFace &face : model.faces)
            dst->faceAttributes.push_back(std::to_underlying(face.attributes & ~FACE_HAS_EVENT));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    snapshot(pActors, &dst->actors);
    snapshot(pSpriteObjects, &dst->spriteObjects);
    snapshot(vChests, &dst->chests);
    snapshot(engine->_persistentVariables, &dst->eventVariables);
    snapshot(src.loc_time, &dst->locationTime);
}

void reconstruct(const OutdoorDelta_MM7 &src, OutdoorLocation *dst) {
    reconstruct(src.header.info, &dst->ddm);
    reconstruct(src.fullyRevealedCells, &dst->uFullyRevealedCellOnMap);
    reconstruct(src.partiallyRevealedCells, &dst->uPartiallyRevealedCellOnMap);

    // Not all of the attributes need to be restored.
    size_t attributeIndex = 0;
    for (BSPModel &model : dst->pBModels) {
        for (ODMFace &face : model.faces) {
            face.attributes &= FACE_HAS_EVENT; // TODO(captainurist): skip FACE_TEXTURE_FRAME here too?
            face.attributes |= FaceAttributes(src.faceAttributes[attributeIndex++]) & ~FACE_HAS_EVENT;
        }
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    reconstruct(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    reconstruct(src.spriteObjects, &pSpriteObjects);

    vChests.resize(src.chests.size());
    for (size_t i = 0; i < src.chests.size(); ++i)
        reconstruct(src.chests[i], &vChests[i], tags::context<int>(i));

    reconstruct(src.eventVariables, &engine->_persistentVariables);
    reconstruct(src.locationTime, &dst->loc_time);
}

void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.fullyRevealedCells, dst);
    serialize(src.partiallyRevealedCells, dst);
    serialize(src.faceAttributes, dst, tags::unsized);
    serialize(src.decorationFlags, dst, tags::unsized);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.eventVariables, dst);
    serialize(src.locationTime, dst);
}

void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx->models)
        totalFaces += model.numFaces;

    deserialize(src, &dst->header);
    deserialize(src, &dst->fullyRevealedCells);
    deserialize(src, &dst->partiallyRevealedCells);
    deserialize(src, &dst->faceAttributes, tags::presized(totalFaces));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->locationTime);
}

void snapshot(const SaveGame &src, SaveGame_MM7 *dst) {
    snapshot(src.header, &dst->header);
    snapshot(src.party, &dst->party);
    snapshot(src.eventTimer, &dst->eventTimer);
    snapshot(src.overlays, &dst->overlays);
    snapshot(src.npcData, &dst->npcData);
    snapshot(src.npcGroups, &dst->npcGroups);

    // Share map deltas.
    dst->mapDeltas.clear();
    for (const auto &[key, value] : src.mapDeltas)
        dst->mapDeltas[key] = Blob::share(value);

    // Encode Lloyd's Beacon images from party.
    dst->lloydImages.clear();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (!src.party.pCharacters[i].vBeacons[j])
                continue;
            const LloydBeacon &beacon = *src.party.pCharacters[i].vBeacons[j];
            if (beacon.uBeaconTime.isValid() && beacon.image != nullptr)
                dst->lloydImages[{i, j}] = pcx::encode(beacon.image->rgba());
        }
    }

    dst->thumbnail = Blob::share(src.thumbnail);
}

void reconstruct(const SaveGame_MM7 &src, SaveGame *dst) {
    reconstruct(src.header, &dst->header);
    reconstruct(src.party, &dst->party);
    reconstruct(src.eventTimer, &dst->eventTimer);
    reconstruct(src.overlays, &dst->overlays);
    reconstruct(src.npcData, &dst->npcData);
    reconstruct(src.npcGroups, &dst->npcGroups);

    // Share map deltas.
    dst->mapDeltas.clear();
    for (const auto &[key, value] : src.mapDeltas)
        dst->mapDeltas[key] = Blob::share(value);

    // Decode Lloyd's Beacon images into party.
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            if (!dst->party.pCharacters[i].vBeacons[j])
                continue;
            LloydBeacon &beacon = *dst->party.pCharacters[i].vBeacons[j];
            beacon.image = GraphicsImage::Create(pcx::decode(src.lloydImages.at({i, j})));
        }
    }

    dst->thumbnail = Blob::share(src.thumbnail);
}

void serialize(const SaveGame_MM7 &src, Blob *dst) {
    LodInfo lodInfo;
    lodInfo.version = LOD_VERSION_MM7;
    lodInfo.rootName = "chapter";
    lodInfo.description = "newmaps for MMVII";

    BlobOutputStream stream(dst);
    LodWriter lodWriter(&stream, std::move(lodInfo));

    lodWriter.write("header.bin", toBlob(src.header));
    lodWriter.write("party.bin", toBlob(src.party));
    lodWriter.write("clock.bin", toBlob(src.eventTimer));
    lodWriter.write("overlay.bin", toBlob(src.overlays));
    lodWriter.write("npcdata.bin", toBlob(src.npcData));
    lodWriter.write("npcgroup.bin", toBlob(src.npcGroups));

    for (const auto &[name, blob] : src.mapDeltas)
        lodWriter.write(name, blob);

    for (const auto &[key, blob] : src.lloydImages)
        lodWriter.write(fmt::format("lloyd{}{}.pcx", key.first + 1, key.second + 1), blob);

    lodWriter.write("image.pcx", src.thumbnail);

    // Apparently vanilla had two bugs canceling each other out:
    // 1. Broken binary search implementation when looking up LOD entries.
    // 2. Writing additional duplicate entry at the end of a saves LOD file.
    // Our code doesn't support duplicate entries, so we just add a dummy entry.
    lodWriter.write("z.bin", Blob::fromString("dummy"));

    lodWriter.close();
    stream.close();
}

void deserialize(const Blob &src, SaveGame_MM7 *dst) {
    LodReader lodReader(Blob::share(src), LOD_ALLOW_DUPLICATES);

    deserialize(lodReader.read("header.bin"), &dst->header);
    deserialize(lodReader.read("party.bin"), &dst->party);
    deserialize(lodReader.read("clock.bin"), &dst->eventTimer);
    deserialize(lodReader.read("overlay.bin"), &dst->overlays);
    deserialize(lodReader.read("npcdata.bin"), &dst->npcData);
    deserialize(lodReader.read("npcgroup.bin"), &dst->npcGroups);

    dst->mapDeltas.clear();
    for (const std::string &name : lodReader.ls())
        if (name.ends_with(".ddm") || name.ends_with(".dlv"))
            dst->mapDeltas[name] = lodReader.read(name);

    dst->lloydImages.clear();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            std::string name = fmt::format("lloyd{}{}.pcx", i + 1, j + 1);
            if (lodReader.exists(name))
                dst->lloydImages[{i, j}] = lodReader.read(name);
        }
    }

    dst->thumbnail = lodReader.read("image.pcx");
}

void reconstruct(const SaveGameLite_MM7 &src, SaveGameLite *dst) {
    reconstruct(src.header, &dst->header);
    dst->thumbnail = Blob::share(src.thumbnail);
}

void deserialize(const Blob &src, SaveGameLite_MM7 *dst) {
    LodReader lodReader(Blob::share(src), LOD_ALLOW_DUPLICATES);
    deserialize(lodReader.read("header.bin"), &dst->header);
    dst->thumbnail = lodReader.read("image.pcx");
}

void reconstruct(const SpriteFrameTable_MM7 &src, SpriteFrameTable *dst) {
    reconstruct(src.frames, &dst->pSpriteSFrames);
    reconstruct(src.eframes, &dst->pSpriteEFrames);
}

void deserialize(InputStream &src, SpriteFrameTable_MM7 *dst) {
    deserialize(src, &dst->frameCount);
    deserialize(src, &dst->eframeCount);
    deserialize(src, &dst->frames, tags::presized(dst->frameCount));
    deserialize(src, &dst->eframes, tags::presized(dst->eframeCount));
}
