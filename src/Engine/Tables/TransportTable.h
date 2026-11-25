#pragma once

#include <array>
#include <vector>

#include "Engine/Data/HouseEnums.h"
#include "Engine/MapEnums.h"
#include "Engine/PartyEnums.h"

#include "Library/Geometry/Vec.h"

#include "Utility/IndexedArray.h"

class Blob;

/**
 * One transport departure, as MM8's own `Data/Tables/Transport Locations.txt` describes it.
 *
 * MM7 ships this data compiled into its executable, so `Transport.cpp` carries a hardcoded copy of
 * it. MM8 ships it as two loose tab-separated tables instead, and nothing read them - which left
 * every MM8 boat and caravan with no destinations at all, and so no way to travel between MM8's
 * regions (measured: "The Windling", the boat off Dagger Wound Island, is house 63, which lands on
 * MM7's `HOUSE_BOAT_EMERALD_ISLAND` row - all four of its routes are the 255 "no route" sentinel).
 */
struct TransportScheduleData {
    MapId mapId = MAP_INVALID;
    std::array<unsigned char, 7> schedule{};  // indexed by `uCurrentDayOfMonth % 7`, same as MM7's
    int travelTimeDays = 0;
    Vec3f arrivalPos;
    int arrivalViewYaw = 0;
    QuestBit questBit = QBIT_INVALID;  // route only offered once this bit is set; QBIT_INVALID = always
};

/** MM8's departures, by the index `transportRoutesMm8` points at. Empty unless MM8 data was loaded. */
extern std::vector<TransportScheduleData> transportScheduleMm8;

/** MM8's per-house routes: up to four departure indices, 255 meaning "this slot has no route". */
extern IndexedArray<std::array<int, 4>, HOUSE_FIRST_TRANSPORT, HOUSE_LAST_TRANSPORT> transportRoutesMm8;

/** True once both MM8 tables have been parsed; `Transport.cpp` falls back to MM7's data when false. */
extern bool transportTableMm8Loaded;

/**
 * Parses MM8's `Transport Index.txt` and `Transport Locations.txt`. Either blob being empty (the
 * tables are not shipped, e.g. an MM7 install) leaves `transportTableMm8Loaded` false and changes
 * nothing.
 */
void initializeTransportMm8(const Blob &index, const Blob &locations);
