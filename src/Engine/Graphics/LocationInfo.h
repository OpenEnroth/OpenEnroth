#pragma once

#include "Engine/Time.h"

struct LocationInfo {
    int respawnCount = 0; // Number of times a location was respawned, including the initial spawn.
    int lastRespawnDay = 0; // Day of the last respawn (days since GameTime zero to last respawn).
    int reputation = 0; // Sign-flipped party reputation in this location, negative value means positive reputation.
    bool alertStatus = false; // Semantics is unclear.
};
