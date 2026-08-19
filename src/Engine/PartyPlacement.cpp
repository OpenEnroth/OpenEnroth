#include "PartyPlacement.h"

#include "Engine/Party.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

void placeParty(const PartyPlacement &placement) {
    const Vec3f &pos = placement.pos;

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        if (!pIndoor->GetSector(pos)) {
            MM_ERROR("placeParty - Cannot GetSector for target position ({}, {}, {}), skipping teleport", pos.x, pos.y, pos.z);
            return;
        }
    } else {
        bool partyIsOnWater = false;
        int floorFaceId = -1;
        float newFloorLevel = ODM_GetFloorLevel(pos, &partyIsOnWater, &floorFaceId);
        if (pos.x < -maxPartyAxisDistance || pos.x > maxPartyAxisDistance ||
            pos.y < -maxPartyAxisDistance || pos.y > maxPartyAxisDistance) {
            MM_ERROR("placeParty - Target position ({}, {}, {}) is out of bounds, skipping teleport", pos.x, pos.y, pos.z);
            return;
        }
        // Warn about teleport height - party will be correctly z positioned on next update
        if (pos.z < newFloorLevel)
            MM_WARNING("placeParty - Target position ({}, {}, {}) is below the floor level of {}", pos.x, pos.y, pos.z, newFloorLevel);
    }

    pParty->pos = pos;
    pParty->velocity = Vec3f(0, 0, placement.zSpeed);
    pParty->uFallStartZ = pos.z;
    if (placement.yaw != -1)
        pParty->_viewYaw = placement.yaw;
    pParty->_viewPitch = placement.pitch;
}

PartyPlacement placementForStartPoint(MapStartPoint point) {
    PartyPlacement result;
    result.pos = pParty->pos;
    result.yaw = pParty->_viewYaw;
    result.pitch = pParty->_viewPitch;

    DecorationId decorationId = pDecorationList->GetDecorIdByName(toString(point));
    if (decorationId == DECORATION_NULL)
        return result;

    for (const LevelDecoration &decoration : pLevelDecorations) {
        if (decoration.uDecorationDescID != decorationId)
            continue;

        result.pos = decoration.vPosition;
        if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            // Spawn point in Harmondale from Barrow Downs is up in the sky, vanilla worked it around by
            // always placing the party on the ground.
            bool onWater = false;
            int bmodelPid = 0;
            result.pos.z = ODM_GetFloorLevel(result.pos, &onWater, &bmodelPid);
        } else {
            int face = -1;
            result.pos.z = BLV_GetFloorLevel(result.pos, pIndoor->GetSector(result.pos), &face);
        }
        result.yaw = decoration._yawAngle;
        result.pitch = 0;
    }

    return result;
}
