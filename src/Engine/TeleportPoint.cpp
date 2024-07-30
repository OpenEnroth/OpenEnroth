#include "TeleportPoint.h"

#include "Engine/Party.h"

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
