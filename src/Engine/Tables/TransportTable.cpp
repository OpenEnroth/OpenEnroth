#include "TransportTable.h"

#include <string>
#include <vector>

#include "Engine/Tables/MapTable.h"

#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"

std::vector<TransportScheduleData> transportScheduleMm8;
IndexedArray<std::array<int, 4>, HOUSE_FIRST_TRANSPORT, HOUSE_LAST_TRANSPORT> transportRoutesMm8 = {};
bool transportTableMm8Loaded = false;

//                         0    1      2       3        4          5         6       7        8
// Transport Locations.txt: #   Map    Monday  Tuesday  Wednesday  Thursday  Friday  Saturday Sunday
//                         9          10 11 12 13         14
//                         DaysCount  X  Y  Z  Direction  QBit
//
// Transport Index.txt: 2D Event (the house id), then four departure indices, -1 for an empty slot.
// The house ids in that table are exactly `HOUSE_FIRST_TRANSPORT..HOUSE_LAST_TRANSPORT` (54..73),
// the same range MM7's own `transportRoutes` covers.
static constexpr int kNoRoute = 255;  // same sentinel MM7's table and `Transport.cpp` already use

/** MM8 names its maps as file names here ("Out02.odm"). `MapTable::GetMapInfo` asserts on an
 *  unknown name, and a table row we cannot resolve should disable that one route, not abort. */
static MapId mapIdByFileName(std::string_view name) {
    std::string lowered = ascii::toLower(name);
    for (MapId id : pMapTable->pInfos.indices())
        if (pMapTable->pInfos[id].fileName == lowered)
            return id;
    return MAP_INVALID;
}

void initializeTransportMm8(const Blob &index, const Blob &locations) {
    transportScheduleMm8.clear();
    transportRoutesMm8 = {};
    transportTableMm8Loaded = false;
    if (!index.size() || !locations.size())
        return;

    for (std::string_view line : split(locations.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 15> tokens = split(line).by('\t').replace("", "0").resize(15, "0");
        TransportScheduleData row;
        row.mapId = mapIdByFileName(tokens[1]);
        for (int day = 0; day < 7; day++)
            row.schedule[day] = tokens[2 + day] == "x" || tokens[2 + day] == "X" ? 1 : 0;
        row.travelTimeDays = fromString<int>(tokens[9]);
        row.arrivalPos = Vec3f(fromString<int>(tokens[10]), fromString<int>(tokens[11]), fromString<int>(tokens[12]));
        row.arrivalViewYaw = fromString<int>(tokens[13]);
        row.questBit = static_cast<QuestBit>(fromString<int>(tokens[14]));
        transportScheduleMm8.push_back(row);
    }
    if (transportScheduleMm8.empty())
        return;

    for (std::string_view line : split(index.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 5> tokens = split(line).by('\t').replace("", "-1").resize(5, "-1");
        int houseId = fromString<int>(tokens[0]);
        if (houseId < std::to_underlying(HOUSE_FIRST_TRANSPORT) || houseId > std::to_underlying(HOUSE_LAST_TRANSPORT))
            continue;
        for (int slot = 0; slot < 4; slot++) {
            int scheduleId = fromString<int>(tokens[1 + slot]);
            bool usable = scheduleId >= 0 && scheduleId < static_cast<int>(transportScheduleMm8.size()) &&
                          transportScheduleMm8[scheduleId].mapId != MAP_INVALID;
            transportRoutesMm8[static_cast<HouseId>(houseId)][slot] = usable ? scheduleId : kNoRoute;
        }
    }
    transportTableMm8Loaded = true;
}
