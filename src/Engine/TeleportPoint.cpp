#include "TeleportPoint.h"

#include "Engine/Party.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "engine/Graphics/LocationFunctions.h"

#include "Library/Logger/Logger.h"

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
            logger->warning("TeleportPoint::doTeleport - Cannot GetSector for target position ({}, {}, {}), skipping teleport", _pos.x, _pos.y, _pos.z);
            return;
        }
    } else {
        bool partyIsOnWater = false;
        int floorFaceId = -1;
        float newFloorLevel = ODM_GetFloorLevel(_pos, &partyIsOnWater, &floorFaceId) + 1;
        if (_pos.x < -maxPartyAxisDistance || _pos.x > maxPartyAxisDistance ||
            _pos.y < -maxPartyAxisDistance || _pos.y > maxPartyAxisDistance ||
            _pos.z < newFloorLevel) {
            logger->warning("TeleportPoint::doTeleport - Target position ({}, {}, {}) is out of bounds, skipping teleport", _pos.x, _pos.y, _pos.z);
            return;
        }
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
