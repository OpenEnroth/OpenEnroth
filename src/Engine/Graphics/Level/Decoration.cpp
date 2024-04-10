#include "Engine/Graphics/Level/Decoration.h"

#include <cstdlib>
#include <algorithm>
#include <vector>

#include "Engine/Party.h"
#include "Engine/Random/Random.h"

#include "Library/Logger/Logger.h"

std::vector<LevelDecoration> pLevelDecorations;
std::vector<int> decorationsWithSound;
LevelDecoration *activeLevelDecoration;

//----- (004583B0) --------------------------------------------------------
LevelDecoration::LevelDecoration() { memset(this, 0, sizeof(*this)); }

//----- (00450929) --------------------------------------------------------
int LevelDecoration::GetGlobalEvent() {
    // LevelDecoration *v1; // esi@1
    // signed int v2; // eax@1
    // int v3; // eax@5
    // int v4; // eax@6
    // int v5; // eax@7
    // int v6; // eax@8
    // int v7; // eax@9
    // int result; // eax@14
    // int v9; // eax@18
    // int v10; // eax@19
    // int v11; // eax@20
    // int v12; // eax@21
    // int v13; // eax@22

    switch (uDecorationDescID) {
        case 0:
        case 1:
        case 2:
        case 3:
            return 0;

        case 4:
            return 16;  // dec01 "Trash Pile"
        case 5:
            return 32;  // dec02 "Campfire"
        case 6:
            return 12 + grng->random(4);  // dec03 "Cauldron"
        case 7:
        case 8:
        case 9:
        case 10:
            return 0;

        case 11:
            return 34;  // dec08 "Fruit plate"
        case 12:
            return 0;

        case 13:
            return 17;  // dec10 "Trash Pile"
        case 14:
            return 18;  // dec11 "Filth"
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            return 0;

        case 24:
            return 36;  // dec21 "Keg"
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
            return 0;

        case 34:
            return 4 + grng->random(6);  // dec32 "Barrel"
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
        case 120:
        case 121:
        case 122:
        case 123:
        case 124:
        case 125:
        case 126:
        case 127:
        case 128:
        case 129:
        case 130:
        case 131:
        case 132:
        case 133:
        case 134:
        case 135:
        case 136:
        case 137:
        case 138:
        case 139:
        case 140:
        case 141:
        case 142:
        case 143:
        case 144:
        case 145:
        case 146:
        case 147:
        case 148:
        case 149:
        case 150:
        case 151:
        case 152:
        case 153:
        case 154:
        case 155:
        case 156:
        case 157:
        case 158:
        case 159:
        case 160:
        case 161:
        case 162:
        case 163:
        case 164:
        case 165:
        case 166:
        case 167:
        case 168:
        case 169:
        case 170:
        case 171:
        case 172:
        case 173:
        case 174:
        case 175:
        case 176:
        case 177:
        case 178:
        case 179:
        case 180:
        case 181:
        case 182:
        case 183:

        case 184:
            return 33;  // dec24 "Campfire"
        case 185:
        case 186:
            return 0;

        case 187:  // dec88 "Mushroom"
        case 190:  // dec91 "Mushroom"
            return 37;

        case 188:
        case 189:
        case 191:
        case 192:
        case 193:
        case 194:
        case 195:
        case 196:
        case 197:
        case 198:
        case 199:
        case 200:
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
            return 0;

        case 206:
            return 162 + grng->random(7);  // dec60
        case 207:
            return 169 + grng->random(7);  // dec61
        case 208:
            return 176 + grng->random(7);  // dec62
        case 209:
            return 183 + grng->random(7);  // dec63
        case 210:
            return 150;  // dec64 "Magic pedestal"
        case 211:
            return 151;  // dec65 "Magic pedestal"
        case 212:
            return 152;  // dec66 "Magic pedestal"
        case 213:
            return 153;  // dec67 "Magic pedestal"
        case 214:
            return 154;  // dec68 "Magic pedestal"
        case 215:
            return 155;  // dec69 "Magic pedestal"
        case 216:
            return 156;  // dec70 "Magic pedestal"
        case 217:
            return 157;  // dec71 "Magic pedestal"
        case 218:
            return 158;  // dec72 "Magic pedestal"
        case 219:
            return 159;  // dec73 "Magic pedestal"
        case 220:
            return 160;  // dec74 "Magic pedestal"
        case 221:
            return 161;  // dec75 "Magic pedestal"

        case 222:
        case 223:
        case 224:
        case 225:
        case 226:
        case 227:
            return 0;

        default:
            logger->error("Invalid DecorationDescID: {}", uDecorationDescID);
            return 0;
    }
}

//----- (0047A825) --------------------------------------------------------
bool LevelDecoration::IsObeliskChestActive() {
    // bool v1; // ebx@1
    // LevelDecoration *v2; // edi@1

    static constexpr std::array<QuestBit, 14> aObeliskQuests = {{
        QBIT_OBELISK_IN_HARMONDALE_FOUND,
        QBIT_OBELISK_IN_ERATHIA_FOUND,
        QBIT_OBELISK_IN_TULAREAN_FOREST_FOUND,
        QBIT_OBELISK_IN_DEYJA_FOUND,
        QBIT_OBELISK_IN_BRACADA_DESERT_FOUND,
        QBIT_OBELISK_IN_CELESTE_FOUND,
        QBIT_OBELISK_IN_THE_PIT_FOUND,
        QBIT_OBELISK_IN_EVENMORN_ISLAND_FOUND,
        QBIT_OBELISK_IN_MOUNT_NIGHON_FOUND,
        QBIT_OBELISK_IN_BARROW_DOWNS_FOUND,
        QBIT_OBELISK_IN_LAND_OF_THE_GIANTS_FOUND,
        QBIT_OBELISK_IN_TATALIA_FOUND,
        QBIT_OBELISK_IN_AVLEE_FOUND,
        QBIT_OBELISK_IN_STONE_CITY_FOUND
    }};

    if (pParty->uCurrentHour == 0 &&
        !TestPartyQuestBit(QBIT_OBELISK_TREASURE_FOUND) &&
        std::all_of(aObeliskQuests.begin(), aObeliskQuests.end(), TestPartyQuestBit)) {
        this->uFlags &= ~LEVEL_DECORATION_INVISIBLE;
        return true;
    }

    this->uFlags |= LEVEL_DECORATION_INVISIBLE;
    return false;
}

//----- (0044C2F4) --------------------------------------------------------
bool LevelDecoration::IsInteractive() {
    switch (uDecorationDescID) {
        case 4:    // trash pile
        case 5:    // campfire
        case 6:    // cauldron
        case 11:   // fruit plate
        case 13:   // trash pile
        case 14:   // dirt
        case 24:   // keg
        case 34:   // barrel
        case 184:  // fire
        case 187:  // fire
        case 190:  // fire
            return true;
    }

    if (uDecorationDescID >= 206 &&
        uDecorationDescID <= 209)  // lighthouse fire
        return true;
    if (uDecorationDescID >= 210 && uDecorationDescID <= 221)  // magic pedestal
        return true;

    return false;
}
