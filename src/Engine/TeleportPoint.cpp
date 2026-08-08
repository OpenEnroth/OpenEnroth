#include "TeleportPoint.h"

#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/LocationFunctions.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

void TeleportPoint::invalidate() {
    _teleportValid = false;
    _targetMap.clear();
}

void TeleportPoint::setTeleportTarget(Vec3f pos, int yaw, int pitch, int zSpeed) {
    _pos = pos;
    _yaw = yaw;
    _pitch = pitch;
    _zSpeed = zSpeed;

    _teleportValid = (pos.lengthSqr() >= 1.0f) || (_yaw != -1) || _pitch || _zSpeed;
}

void TeleportPoint::doTeleport(bool keepOnZero) {
    assert(_teleportValid);

    // Test target position is valid
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        if (!pIndoor->GetSector(_pos)) {
            MM_ERROR("TeleportPoint::doTeleport - Cannot GetSector for target position ({}, {}, {}), skipping teleport", _pos.x, _pos.y, _pos.z);
            return;
        }
    } else {
        bool partyIsOnWater = false;
        int floorFaceId = -1;
        float newFloorLevel = ODM_GetFloorLevel(_pos, &partyIsOnWater, &floorFaceId);
        if (_pos.x < -maxPartyAxisDistance || _pos.x > maxPartyAxisDistance ||
            _pos.y < -maxPartyAxisDistance || _pos.y > maxPartyAxisDistance ) {
            MM_ERROR("TeleportPoint::doTeleport - Target position ({}, {}, {}) is out of bounds, skipping teleport", _pos.x, _pos.y, _pos.z);
            return;
        }
        // Warn about teleport height - party will be correctly z positioned on next update
        if (_pos.z < newFloorLevel)
            MM_WARNING("TeleportPoint::doTeleport - Target position ({}, {}, {}) is below the floor level of {}", _pos.x, _pos.y, _pos.z, newFloorLevel);
    }

    Vec3f newPos = pParty->pos;
    Vec3f newSpeed = pParty->velocity;
    int newFallStart = pParty->uFallStartZ;
    int newPitch = pParty->_viewPitch;

    if (keepOnZero) {
        if (_pos.x) {
            newPos.x = _pos.x;
        }
        if (_pos.y) {
            newPos.y = _pos.y;
        }
        if (_pos.z) {
            newPos.z = _pos.z;
            newFallStart = _pos.z;
        }
        if (_zSpeed) {
            newSpeed = Vec3f(0, 0, _zSpeed);
        }
        if (_pitch) {
            newPitch = _pitch;
        }
    } else {
        newPos = _pos;
        newSpeed = Vec3f(0, 0, _zSpeed);
        newFallStart = _pos.z;
        newPitch = _pitch;
    }

    pParty->pos = newPos;
    pParty->velocity = newSpeed;
    pParty->uFallStartZ = newFallStart;
    if (_yaw != -1) {
        pParty->_viewYaw = _yaw;
    }
    pParty->_viewPitch = newPitch;
}

MapStartPoint uLevel_StartingPointType;

void TeleportToStartingPoint(MapStartPoint point) {
    DecorationId decID = pDecorationList->GetDecorIdByName(toString(point));

    if (decID != DECORATION_NULL) {
        for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
            if (pLevelDecorations[i].uDecorationDescID == decID) {
                pParty->pos = pLevelDecorations[i].vPosition;
                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                    // Spawn point in Harmondale from Barrow Downs is up in the sky, vanilla worked it around by
                    // always placing the party on the ground.
                    // TODO: (Chaosit) dummy variables created for the sake of passing pointers
                    bool bOnWater = false;
                    int bModelPid;
                    pParty->pos.z = ODM_GetFloorLevel(pParty->pos, &bOnWater, &bModelPid);
                } else {
                    int face = -1;
                    pParty->pos.z = BLV_GetFloorLevel(pParty->pos, pIndoor->GetSector(pParty->pos), &face);
                }
                pParty->velocity = Vec3f();
                pParty->uFallStartZ = pParty->pos.z;
                pParty->_viewYaw = pLevelDecorations[i]._yawAngle;
                pParty->_viewPitch = 0;
            }
        }

        if (engine->_teleportPoint.isValid()) {
            engine->_teleportPoint.doTeleport(true);
        }
        engine->_teleportPoint.invalidate();
    }
}
