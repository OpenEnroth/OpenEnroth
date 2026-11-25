#include "PartyStep.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "Engine/Engine.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/Collisions.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/Character.h"
#include "Engine/Party.h"
#include "Engine/Timer.h"
#include "Engine/mm7_data.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

namespace {

bool g_dryRun = false;
bool g_dryRunIgnoresActors = false;
PartyMotionEffects *g_effects = nullptr;

// Every field the two movement ticks write on the party and its globals (`ODM_ProcessPartyActions`,
// `BLV_ProcessPartyActions`, `ProcessPartyCollisions*`) - captured before a dry run, put back after it.
struct SavedPartyMotion {
    Vec3f pos;
    Vec3f velocity;
    int floorFaceId;
    int fallStartZ;
    int savedFlightZ;
    int viewYaw;
    int viewPitch;
    PartyFlags flags;
    unsigned int flying;
    SoundId walkingSound;
    int partySectorId;
    CollisionState collision;
    unsigned char waterWalkOverlay;
    unsigned char flyOverlay;

    static int overlayIndex(PartyBuff buff) {
        return 20 * pParty->pPartyBuffs[buff].overlayId + 119;
    }

    static SavedPartyMotion capture() {
        SavedPartyMotion s;
        s.pos = pParty->pos;
        s.velocity = pParty->velocity;
        s.floorFaceId = pParty->floor_face_id;
        s.fallStartZ = pParty->uFallStartZ;
        s.savedFlightZ = pParty->sPartySavedFlightZ;
        s.viewYaw = pParty->_viewYaw;
        s.viewPitch = pParty->_viewPitch;
        s.flags = pParty->uFlags;
        s.flying = pParty->bFlying;
        s.walkingSound = pParty->currentWalkingSound;
        s.partySectorId = pBLVRenderParams ? pBLVRenderParams->uPartySectorID : 0;
        s.collision = collision_state;
        s.waterWalkOverlay = engine->_persistentVariables.decorVars[overlayIndex(PARTY_BUFF_WATER_WALK)];
        s.flyOverlay = engine->_persistentVariables.decorVars[overlayIndex(PARTY_BUFF_FLY)];
        return s;
    }

    void restore() const {
        pParty->pos = pos;
        pParty->velocity = velocity;
        pParty->floor_face_id = floorFaceId;
        pParty->uFallStartZ = fallStartZ;
        pParty->sPartySavedFlightZ = savedFlightZ;
        pParty->_viewYaw = viewYaw;
        pParty->_viewPitch = viewPitch;
        pParty->uFlags = flags;
        pParty->bFlying = flying;
        pParty->currentWalkingSound = walkingSound;
        if (pBLVRenderParams)
            pBLVRenderParams->uPartySectorID = partySectorId;
        collision_state = collision;
        engine->_persistentVariables.decorVars[overlayIndex(PARTY_BUFF_WATER_WALK)] = waterWalkOverlay;
        engine->_persistentVariables.decorVars[overlayIndex(PARTY_BUFF_FLY)] = flyOverlay;
    }
};

// The floor the engine puts under `pos` - outdoors the highest floor at or below it (terrain or a model face),
// indoors the sector/face the party would stand on. Returns false when there is none (outside the level).
bool floorUnder(const Vec3f &pos, float *floorZ, int *faceId, int *sectorId, bool *onWater) {
    *faceId = -1;
    *sectorId = 0;
    *onWater = false;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        if (!pIndoor)
            return false;
        int sector = pIndoor->GetSector(pos);
        if (sector == 0)
            return false;
        int face = -1;
        float z = GetIndoorFloorZ(pos + Vec3f(0, 0, pParty->radius), &sector, &face);
        if (z == -30000 || face == -1)
            return false;
        *floorZ = z;
        *faceId = face;
        *sectorId = sector;
        return true;
    }
    if (!pOutdoor)
        return false;
    *floorZ = ODM_GetFloorLevel(pos, onWater, faceId);
    return true;
}

// The collision pass reports the floor the party stands on as a contact every frame indoors (the feet
// sphere rests on it); that is standing, not being stopped. Only a wall, a ceiling, a decoration or a
// body counts as something the party pressed against.
bool isFloorContact(Pid pid) {
    if (!pid || pid.type() != OBJECT_Face)
        return false;
    const BLVFace *face = nullptr;
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        if (pIndoor && pid.id() >= 0 && static_cast<size_t>(pid.id()) < pIndoor->faces.size())
            face = &pIndoor->faces[pid.id()];
    } else if (pOutdoor) {
        if (pid.id() >= 0 && static_cast<size_t>(pid.id()) < pOutdoor->faces.size())
            face = &pOutdoor->faces[pid.id()];
    }
    return face && (face->polygonType == POLYGON_Floor || face->polygonType == POLYGON_InBetweenFloorAndWall);
}

void runOneMovementTick() {
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        BLV_ProcessPartyActions();
    else
        ODM_ProcessPartyActions();
}

// The outdoor map's edge as the game itself applies it right after the movement tick
// (`ODM_UpdateUserInputAndOther`): beyond `maxPartyAxisDistance` the party is clamped back or offered
// travel. The terrain has no floor to query past there, so the dry run clamps the same way and stops.
bool outsideOutdoorMap(const Vec3f &pos) {
    return uCurrentlyLoadedLevelType == LEVEL_OUTDOOR &&
           (std::abs(pos.x) > maxPartyAxisDistance || std::abs(pos.y) > maxPartyAxisDistance);
}

} // anonymous namespace

float PartyStepResult::planarTravel() const {
    float dx = end.x - start.x, dy = end.y - start.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool partyMotionDryRun() {
    return g_dryRun;
}

static bool g_seamFloorResolve = false;
void setSeamFloorResolve(bool on) {
    g_seamFloorResolve = on;
}
bool seamFloorResolve() {
    return g_seamFloorResolve;
}

bool partyMotionDryRunIgnoresActors() {
    return g_dryRun && g_dryRunIgnoresActors;
}

void partyMotionFaceEvent(int eventId) {
    if (g_dryRun) {
        g_effects->faceEvent = eventId;
        return;
    }
    runEvent(EvtContext(EVT_SOURCE_MAP, eventId));
}

void partyMotionFallDamage(int distance) {
    if (g_dryRun) {
        g_effects->fallDamageDistance = std::max(g_effects->fallDamageDistance, distance);
        return;
    }
    pParty->giveFallDamage(distance);
}

void partyMotionFallingScream() {
    if (g_dryRun) {
        g_effects->fallingScream = true;
        return;
    }
    for (Character &character : pParty->pCharacters) {
        if (!character.wearsEnchantedItem(ITEM_ENCHANTMENT_OF_FEATHER_FALLING) &&
            !character.wearsItem(ITEM_ARTIFACT_HERMES_SANDALS) &&
            character.CanAct()) {
            character.playReaction(SPEECH_FALLING);
        }
    }
}

void partyMotionSplash() {
    if (g_dryRun) {
        g_effects->splash = true;
        return;
    }
    // Party can never see its own splashes so just play the sound - only one splash at a time for party.
    pAudioPlayer->playSound(SOUND_splash, SOUND_MODE_EXCLUSIVE, Pid::character(0));
}

// The terrain height query asserts on a pose exactly on a tile corner (`OutdoorTerrain::heightByPos`
// picks the branch where the in-tile offset comes out as a whole tile). No walking party ever sits on
// one to the float; a synthetic cell centre can. Half a unit off the seam is the same ground.
Vec3f offTileCorner(const Vec3f &pos) {
    if (uCurrentlyLoadedLevelType != LEVEL_OUTDOOR)
        return pos;
    Vec3f p = pos;
    if (std::fmod(p.x, 512.0f) == 0.0f)
        p.x += 0.5f;
    if (std::fmod(p.y, 512.0f) == 0.0f)
        p.y += 0.5f;
    return p;
}

PartyStepResult simulatePartySteps(const Vec3f &fromRaw, int viewYaw, const PartyStepInput &input, int ticks,
                                   int settleTicks, const PartyStepTraceFn &trace) {
    assert(!g_dryRun); // A dry run never nests - the tick it drives is not re-entrant either.

    const Vec3f from = offTileCorner(fromRaw);
    PartyStepResult result;
    result.start = from;
    result.end = from;

    float floorZ = 0.0f;
    int faceId = -1, sectorId = 0;
    bool onWater = false;
    if (outsideOutdoorMap(from) || !floorUnder(from, &floorZ, &faceId, &sectorId, &onWater)) {
        result.noFloor = true;
        return result;
    }

    SavedPartyMotion saved = SavedPartyMotion::capture();
    g_dryRun = true;
    g_dryRunIgnoresActors = input.ignoreActors;
    g_effects = &result.effects;

    // A dry run is a run of ordinary frames. The timer's last dt is whatever the last frame was - after a
    // time skip (a boat trip, a rest) that is the skip itself, and a key held for one such "frame" would
    // fling the party across the map. Pin it to the frame the engine is configured to run at.
    Duration savedDt = gameTimer->dt();
    Duration frameDt = savedDt;
    if (frameDt.realtimeMilliseconds() <= 0 || frameDt.realtimeMilliseconds() > 1000) {
        int configuredMs = engine->config->debug.TraceFrameTimeMs.value();
        frameDt = Duration::fromRealtimeMilliseconds(configuredMs > 0 ? configuredMs : 16);
        gameTimer->setDt(frameDt);
    }
    bool running = pParty->uFlags2 & PARTY_FLAGS_2_RUNNING;  // KeyboardInputHandler::ProcessGameplayAction
    result.expectedTravelPerTick = frameDt.realtimeMillisecondsFloat() * pParty->walkSpeed * fWalkSpeedMultiplier * (running ? 2.0f : 1.0f);

    // At rest on the floor under `from`, facing `viewYaw`, with none of the transient motion flags of wherever the
    // real party happens to be right now.
    pPartyActionQueue->Reset();
    pParty->pos = from;
    pParty->velocity = Vec3f(0, 0, 0);
    pParty->floor_face_id = faceId;
    pParty->uFallStartZ = from.z;
    pParty->sPartySavedFlightZ = from.z;
    pParty->bFlying = 0;
    pParty->_viewYaw = viewYaw;
    pParty->uFlags &= ~(PARTY_FLAG_AIRBORNE | PARTY_FLAG_LANDING | PARTY_FLAG_JUMPING |
                        PARTY_FLAG_STANDING_ON_WATER | PARTY_FLAG_WATER_DAMAGE | PARTY_FLAG_BURNING);
    if (pBLVRenderParams && uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        pBLVRenderParams->uPartySectorID = sectorId;

    bool released = false;
    Vec3f framePrev = pParty->pos;
    auto observe = [&](int tick, bool keysHeld) {
        result.ticks = tick + 1;
        // The FIRST thing the party pressed against while the keys were held - what stopped it. A
        // later contact is the slide along it (a party stopped by a closed door slides onto the wall
        // beside it; the door is what it hit, and what its caller needs to know about).
        // The horizontal pass's contact: the vertical pass that follows it overwrites `pid` with the
        // floor the party lands on, every frame (measured: a party pressed against a closed door
        // reported only the floor face under it). And a contact counts as what STOPPED the party
        // only on a frame that went less than half its way - brushing a wall while still making
        // ground is a slide, not a stop (the same half rule every probe is judged by).
        Pid contact = collision_state.horizontalPid ? collision_state.horizontalPid : collision_state.pid;
        float frameDx = pParty->pos.x - framePrev.x, frameDy = pParty->pos.y - framePrev.y;
        bool stoppedThisFrame = std::sqrt(frameDx * frameDx + frameDy * frameDy) < 0.5f * result.expectedTravelPerTick;
        framePrev = pParty->pos;
        if (keysHeld && contact && stoppedThisFrame && !result.collidedWith && !isFloorContact(contact)) {
            result.collidedWith = contact;
            result.collidedAt = pParty->pos;
        }
        if (outsideOutdoorMap(pParty->pos)) {
            pParty->pos.x = std::clamp(pParty->pos.x, -maxPartyAxisDistance, maxPartyAxisDistance);
            pParty->pos.y = std::clamp(pParty->pos.y, -maxPartyAxisDistance, maxPartyAxisDistance);
            result.leftMap = true;
            released = true;
        }
        if (trace && !trace(tick, pParty->pos, pParty->isAirborne(), result.effects))
            released = true;
    };

    // A pose the tick could not have produced (not a number) ends the run before the terrain query asserts on it.
    auto suspicious = [&](int tick, const char *phase) {
        const Vec3f &p = pParty->pos;
        if (std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z))
            return;
        MM_WARNING("[PartyStep] {} tick {}: pos ({},{},{}) is not finite - from ({},{},{}) yaw {} forward={} jump={}",
                        phase, tick, p.x, p.y, p.z, from.x, from.y, from.z, viewYaw, input.forward, input.jump);
        released = true;
        result.noFloor = true;
    };

    int tick = 0;
    for (; tick < ticks && !released; tick++) {
        suspicious(tick, "input");
        if (released)
            break;
        pPartyActionQueue->Reset();
        if (input.forward)
            pPartyActionQueue->Add(running ? PARTY_RunForward : PARTY_WalkForward);
        if (input.backward)
            pPartyActionQueue->Add(running ? PARTY_RunBackward : PARTY_WalkBackward);
        if (input.jump)
            pPartyActionQueue->Add(PARTY_Jump);
        runOneMovementTick();
        observe(tick, /*keysHeld=*/true);
    }
    int groundedTicks = 0;
    for (int settle = 0; settle < settleTicks && !result.leftMap && !result.noFloor; settle++, tick++) {
        suspicious(tick, "settle");
        pPartyActionQueue->Reset();
        runOneMovementTick();
        observe(tick, /*keysHeld=*/false);
        groundedTicks = pParty->isAirborne() ? 0 : groundedTicks + 1;
        if (groundedTicks >= 2)
            break;
    }

    result.end = pParty->pos;
    result.airborne = pParty->isAirborne();
    floorUnder(result.end, &floorZ, &result.floorFaceId, &result.sectorId, &result.onWater);

    pPartyActionQueue->Reset();
    gameTimer->setDt(savedDt);
    g_effects = nullptr;
    g_dryRunIgnoresActors = false;
    g_dryRun = false;
    saved.restore();
    return result;
}
