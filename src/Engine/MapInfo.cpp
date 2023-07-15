#include <cstdlib>
#include <sstream>

#include "Engine/MapInfo.h"
#include "Engine/GameResourceManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"

#include "Library/Serialization/EnumSerialization.h"

#include "Utility/String.h"

#include "Party.h"

const char *location_type[] = {
    "GENERIC",
    "PADDEDCELL",
    "ROOM",
    "BATHROOM",
    "LIVINGROOM",
    "STONEROOM",
    "AUDITORIUM",
    "CONCERTHALL",
    "CAVE",
    "ARENA",
    "HANGAR",
    "CARPETEDHALLWAY",
    "HALLWAY",
    "STONECORRIDOR",
    "ALLEY",
    "FOREST",
    "CITY",
    "MOUNTAIN",
    "QUARRY",
    "PLAINS",
    "PARKINGLOT",
    "SEWERPIPE",
    "UNDERWATER",
    "DRUGGED",
    "DIZZY",
    "PSYCHOTIC"
};

void MapStats::Initialize() {
    std::string pMapStatsTXT = std::string(engine->_gameResourceManager->getEventsFile("MapStats.txt").string_view());
    std::stringstream stream(pMapStatsTXT);
    std::string tmpString;
    std::getline(stream, tmpString);
    std::getline(stream, tmpString);
    std::getline(stream, tmpString);

    char work_str[32];
    int work_str_pos;
    int work_str_len;

    size_t i = 1;
    while (!stream.eof()) {
        std::getline(stream, tmpString);
        std::stringstream line(tmpString);
        size_t decode_step = 0;
        while (!line.eof()) {
            std::getline(line, tmpString, '\t');
            char test_string[1024];
            strncpy(test_string, tmpString.c_str(), sizeof(test_string) - 1);
            switch (decode_step) {
                case 1:
                    pInfos[i].pName = removeQuotes(test_string);  // randoms crashes here  // got 1 too
                    break;
                case 2:
                    pInfos[i].pFilename = toLower(removeQuotes(test_string));
                    break;
                case 3:
                    pInfos[i].uNumResets = atoi(test_string);
                    break;
                case 4:
                    pInfos[i].uFirstVisitedAt = atoi(test_string);
                    break;
                case 5:
                    pInfos[i]._per = atoi(test_string);
                    break;
                case 6:
                    pInfos[i].uRespawnIntervalDays = atoi(test_string);
                    break;
                case 7:
                    pInfos[i]._alert_days = atoi(test_string);
                    break;
                case 8:
                    pInfos[i]._steal_perm = atoi(test_string);
                    break;
                case 9:
                    pInfos[i].LockX5 = atoi(test_string);
                    break;
                case 10:
                    pInfos[i].Trap_D20 = atoi(test_string);
                    break;
                case 11:
                    pInfos[i].Treasure_prob = MAP_TREASURE_LEVEL(atoi(test_string));  // treasure levels 0-6
                    break;
                case 12:
                    pInfos[i].Encounter_percent = atoi(test_string);
                    break;
                case 13:
                    pInfos[i].EncM1percent = atoi(test_string);
                    break;
                case 14:
                    pInfos[i].EncM2percent = atoi(test_string);
                    break;
                case 15:
                    pInfos[i].EncM3percent = atoi(test_string);
                    break;
                case 16:
                    pInfos[i].pEncounterMonster1Texture = removeQuotes(test_string);
                    break;
                case 18:
                    pInfos[i].Dif_M1 = atoi(test_string);
                    break;
                case 19:
                    pInfos[i].uEncounterMonster1AtLeast = 1;
                    pInfos[i].uEncounterMonster1AtMost = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
                    work_str_pos = 0;
                    work_str_len = strlen(work_str);
                    if (work_str_len) {
                        while (work_str[work_str_pos] != '-') {
                            ++work_str_pos;
                            if (work_str_pos >= work_str_len) break;
                        }
                        work_str[work_str_pos] = 0;
                        pInfos[i].uEncounterMonster1AtLeast = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].uEncounterMonster1AtMost = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].uEncounterMonster1AtMost = pInfos[i].uEncounterMonster1AtLeast;
                    }
                    break;
                case 20:
                    pInfos[i].pEncounterMonster2Texture = removeQuotes(test_string);
                    break;
                case 22:
                    pInfos[i].Dif_M2 = atoi(test_string);
                    break;
                case 23:
                    pInfos[i].uEncounterMonster2AtLeast = 1;
                    pInfos[i].uEncounterMonster2AtMost = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
                    work_str_pos = 0;
                    work_str_len = strlen(work_str);
                    if (work_str_len) {
                        while (work_str[work_str_pos] != '-') {
                            ++work_str_pos;
                            if (work_str_pos >= work_str_len) break;
                        }
                        work_str[work_str_pos] = 0;
                        pInfos[i].uEncounterMonster2AtLeast = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].uEncounterMonster2AtMost = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].uEncounterMonster2AtMost = pInfos[i].uEncounterMonster2AtLeast;
                    }
                    break;
                case 24:
                    pInfos[i].pEncounterMonster3Texture = removeQuotes(test_string);
                    break;
                case 26:
                    pInfos[i].Dif_M3 = atoi(test_string);
                    break;
                case 27:
                    pInfos[i].uEncounterMonster3AtLeast = 1;
                    pInfos[i].uEncounterMonster3AtMost = 1;
                    strncpy(work_str, test_string, sizeof(work_str) - 1);
                    work_str_pos = 0;
                    work_str_len = strlen(work_str);
                    if (work_str_len) {
                        while (work_str[work_str_pos] != '-') {
                            ++work_str_pos;
                            if (work_str_pos >= work_str_len) break;
                        }
                        work_str[work_str_pos] = 0;
                        pInfos[i].uEncounterMonster3AtLeast = atoi(work_str);
                        if (work_str_pos < work_str_len)
                            pInfos[i].uEncounterMonster3AtMost = atoi(&work_str[work_str_pos + 1]);
                        else
                            pInfos[i].uEncounterMonster3AtMost = pInfos[i].uEncounterMonster3AtLeast;
                    }
                    break;
                case 28:
                    pInfos[i].uRedbookTrackID = atoi(test_string);
                    break;
                case 29: {
                    pInfos[i].uEAXEnv = 0xff;
                    for (int j = 0; j < 25; j++) {
                        if (!strcmp(test_string, location_type[j])) {
                            pInfos[i].uEAXEnv = j;
                            break;
                        }
                    }
                    if (pInfos[i].uEAXEnv == 0xff) {
                        pInfos[i].uEAXEnv = 26;
                    }
                } break;
            }
            decode_step++;
        }
        i++;
    }

    uNumMaps = i - 1;
}

int MapStats::sub_410D99_get_map_index(int a1) {
    std::string name = pGames_LOD->GetSubNodeName(a1);

    for (int i = 1; i <= pMapStats->uNumMaps; i++) {
        if (pMapStats->pInfos[i].pFilename == name) {
            return i;
        }
    }
    Error("Map not found");
    return -1;
}

MAP_TYPE MapStats::GetMapInfo(const std::string &Str2) {
    Assert(uNumMaps >= 2);

    std::string map_name = toLower(Str2);

    for (uint i = 1; i < uNumMaps; ++i) {
        if (pInfos[i].pFilename == map_name) {
            return (MAP_TYPE)i;
        }
    }

    Error("Map not found!");
    return (MAP_TYPE)-1;  // @TODO: This should be MAP_INVALID!, as it's if'ed later.
}

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(MapStartPoint, CASE_SENSITIVE, {
    {MapStartPoint_Party, "Party Start"},
    {MapStartPoint_North, "North Start"},
    {MapStartPoint_South, "South Start"},
    {MapStartPoint_East, "East Start"},
    {MapStartPoint_West, "West Start"}
})

void TeleportToStartingPoint(MapStartPoint point) {
    std::string pName = toString(point);

    if (pDecorationList->GetDecorIdByName(pName)) {
        if (!pLevelDecorations.empty()) {
            for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
                if (pLevelDecorations[i].uDecorationDescID == pDecorationList->GetDecorIdByName(pName)) {
                    pParty->pos = pLevelDecorations[i].vPosition;
                    pParty->speed = Vec3i();
                    pParty->uFallStartZ = pParty->pos.z;
                    pParty->_viewYaw = (TrigLUT.uIntegerHalfPi * pLevelDecorations[i].field_1A) / 90;
                    if (pLevelDecorations[i]._yawAngle)
                        pParty->_viewYaw = pLevelDecorations[i]._yawAngle;
                    pParty->_viewPitch = 0;
                }
            }
        }
        if (Start_Party_Teleport_Flag) {
            if (Party_Teleport_X_Pos)
                pParty->pos.x = Party_Teleport_X_Pos;
            if (Party_Teleport_Y_Pos)
                pParty->pos.y = Party_Teleport_Y_Pos;
            if (Party_Teleport_Z_Pos) {
                pParty->pos.z = Party_Teleport_Z_Pos;
                pParty->uFallStartZ = Party_Teleport_Z_Pos;
            }
            if (Party_Teleport_Cam_Yaw != -1)
                pParty->_viewYaw = Party_Teleport_Cam_Yaw;
            if (Party_Teleport_Cam_Pitch)
                pParty->_viewPitch = Party_Teleport_Cam_Pitch;
            if (Party_Teleport_Z_Speed)
                pParty->speed = Vec3i(0, 0, Party_Teleport_Z_Speed);
        }
        Party_Teleport_Cam_Yaw = -1;
        Start_Party_Teleport_Flag = 0;
        Party_Teleport_Z_Speed = 0;
        Party_Teleport_Cam_Pitch = 0;
        Party_Teleport_Z_Pos = 0;
        Party_Teleport_Y_Pos = 0;
        Party_Teleport_X_Pos = 0;
    }
}
