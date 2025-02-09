#include "Decoration.h"

#include <cstdlib>
#include <algorithm>
#include <vector>

#include "Engine/Random/Random.h"
#include "Engine/Party.h"

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
        case DECORATION_NULL:
        case DECORATION_PENDING_1:
        case DECORATION_PARTY_START_2:
        case DECORATION_FOUNTAIN_3:
            return 0;

        case DECORATION_TRASH_HEAP_4:
            return 16;  // dec01 "Trash Pile"
        case DECORATION_CAMPFIRE_5:
            return 32;  // dec02 "Campfire"
        case DECORATION_CAULDRON_6:
            return 12 + grng->random(4);  // dec03 "Cauldron"
        case DECORATION_CHANDELIER_7:
        case DECORATION_CAGE_8:
        case DECORATION_CAGE_9:
        case DECORATION_CAGE_10:
            return 0;

        case DECORATION_FRUIT_BOWL_11:
            return 34;  // dec08 "Fruit plate"
        case DECORATION_ROCK_12:
            return 0;

        case DECORATION_TRASH_HEAP_13:
            return 17;  // dec10 "Trash Pile"
        case DECORATION_DIRT_14:
            return 18;  // dec11 "Filth"
        case DECORATION_WOOD_15:
        case DECORATION_WOOD_16:
        case DECORATION_WOOD_17:
        case DECORATION_WOOD_18:
        case DECORATION_WOOD_19:
        case DECORATION_WOOD_20:
        case DECORATION_BONES_21:
        case DECORATION_BONES_22:
        case DECORATION_SKULL_23:
            return 0;

        case DECORATION_CASK_24:
            return 36;  // dec21 "Keg"
        case DECORATION_SHIP_25:
        case DECORATION_BAG_26:
        case DECORATION_BARREL_27:
        case DECORATION_KEG_28:
        case DECORATION_FIRE_29:
        case DECORATION_FIRE_30:
        case DECORATION_TORCH_31:
        case DECORATION_TORCH_32:
        case DECORATION_TORCH_33:
            return 0;

        case DECORATION_BARREL_34:
            return 4 + grng->random(6);  // dec32 "Barrel"
        case DECORATION_BUOY_35:
        case DECORATION_ROCK_36:
        case DECORATION_FOUNTAIN_37:
        case DECORATION_FIRE_38:
        case DECORATION_NORTH_START_39:
        case DECORATION_SOUTH_START_40:
        case DECORATION_EAST_START_41:
        case DECORATION_WEST_START_42:
        case DECORATION_BOTTLE_43:
        case DECORATION_BOWL_44:
        case DECORATION_STACK_OF_BOWLS_45:
        case DECORATION_BOWL_46:
        case DECORATION_STACK_OF_BOWLS_47:
        case DECORATION_GOBLET_48:
        case DECORATION_GOBLET_49:
        case DECORATION_GOBLET_50:
        case DECORATION_MILE_MARKET_51:
        case DECORATION_PLATE_52:
        case DECORATION_STACK_OF_PLATES_53:
        case DECORATION_PLATE_54:
        case DECORATION_STACK_OF_PLATES_55:
        case DECORATION_TREE_56:
        case DECORATION_TREE_57:
        case DECORATION_TREE_58:
        case DECORATION_TREE_59:
        case DECORATION_TREE_60:
        case DECORATION_TREE_61:
        case DECORATION_TREE_62:
        case DECORATION_TREE_63:
        case DECORATION_TREE_64:
        case DECORATION_TREE_65:
        case DECORATION_TREE_66:
        case DECORATION_TREE_67:
        case DECORATION_TREE_68:
        case DECORATION_TREE_69:
        case DECORATION_TREE_70:
        case DECORATION_TREE_71:
        case DECORATION_TREE_72:
        case DECORATION_TREE_73:
        case DECORATION_TREE_74:
        case DECORATION_TREE_75:
        case DECORATION_TREE_76:
        case DECORATION_TREE_77:
        case DECORATION_TREE_78:
        case DECORATION_TREE_79:
        case DECORATION_TREE_80:
        case DECORATION_TREE_81:
        case DECORATION_TREE_82:
        case DECORATION_TREE_83:
        case DECORATION_TREE_84:
        case DECORATION_TREE_85:
        case DECORATION_TREE_86:
        case DECORATION_TREE_87:
        case DECORATION_TREE_88:
        case DECORATION_TREE_89:
        case DECORATION_TREE_90:
        case DECORATION_TREE_91:
        case DECORATION_TREE_92:
        case DECORATION_TREE_93:
        case DECORATION_TREE_94:
        case DECORATION_TREE_95:
        case DECORATION_TREE_96:
        case DECORATION_TREE_97:
        case DECORATION_SND_BROOK_98:
        case DECORATION_SND_BOAT_99:
        case DECORATION_SND_HOWL_100:
        case DECORATION_SND_DRAGON_101:
        case DECORATION_BUSH_102:
        case DECORATION_BUSH_103:
        case DECORATION_BUSH_104:
        case DECORATION_BUSH_105:
        case DECORATION_BUSH_106:
        case DECORATION_BUSH_107:
        case DECORATION_BUSH_108:
        case DECORATION_BUSH_109:
        case DECORATION_ROCK_110:
        case DECORATION_ROCK_111:
        case DECORATION_ROCK_112:
        case DECORATION_ROCK_113:
        case DECORATION_ROCK_114:
        case DECORATION_ROCK_115:
        case DECORATION_ROCK_116:
        case DECORATION_ROCK_117:
        case DECORATION_ROCK_118:
        case DECORATION_ROCK_119:
        case DECORATION_ROCK_120:
        case DECORATION_ROCK_121:
        case DECORATION_ROCK_122:
        case DECORATION_ROCK_123:
        case DECORATION_ROCK_124:
        case DECORATION_ROCK_125:
        case DECORATION_ROCK_126:
        case DECORATION_ROCK_127:
        case DECORATION_ROCK_128:
        case DECORATION_ROCK_129:
        case DECORATION_TORCH_130:
        case DECORATION_TORCH_131:
        case DECORATION_TORCH_132:
        case DECORATION_BUSH_133:
        case DECORATION_BUSH_134:
        case DECORATION_BUSH_135:
        case DECORATION_BUSH_136:
        case DECORATION_BUSH_137:
        case DECORATION_BUSH_138:
        case DECORATION_BUSH_139:
        case DECORATION_BUSH_140:
        case DECORATION_FLOWER_141:
        case DECORATION_FLOWER_142:
        case DECORATION_FLOWER_143:
        case DECORATION_FLOWER_144:
        case DECORATION_FLOWER_145:
        case DECORATION_FLOWER_146:
        case DECORATION_FLOWER_147:
        case DECORATION_FLOWER_148:
        case DECORATION_FLOWER_149:
        case DECORATION_FLOWER_150:
        case DECORATION_FLOWER_151:
        case DECORATION_FLOWER_152:
        case DECORATION_FLOWER_153:
        case DECORATION_FLOWER_154:
        case DECORATION_FLOWER_155:
        case DECORATION_FLOWER_156:
        case DECORATION_PLANT_157:
        case DECORATION_PLANT_158:
        case DECORATION_PLANT_159:
        case DECORATION_PLANT_160:
        case DECORATION_PLANT_161:
        case DECORATION_PLANT_162:
        case DECORATION_PLANT_163:
        case DECORATION_PLANT_164:
        case DECORATION_PLANT_165:
        case DECORATION_PLANT_166:
        case DECORATION_PLANT_167:
        case DECORATION_PLANT_168:
        case DECORATION_PLANT_169:
        case DECORATION_PLANT_170:
        case DECORATION_PLANT_171:
        case DECORATION_PLANT_172:
        case DECORATION_PLANT_173:
        case DECORATION_PLANT_174:
        case DECORATION_PLANT_175:
        case DECORATION_PLANT_176:
        case DECORATION_PLANT_177:
        case DECORATION_PLANT_178:
        case DECORATION_PLANT_179:
        case DECORATION_PLANT_180:
        case DECORATION_STATUE_181:
        case DECORATION_STATUE_182:
        case DECORATION_VASE_183:
            return 0; // This return wasn't here, and looks like this was a bug.

        case DECORATION_FIRE_184:
            return 33;  // dec24 "Campfire"
        case DECORATION_MUSHROOM_185:
        case DECORATION_LOG_186:
            return 0;

        case DECORATION_MUSHROOM_187:  // dec88 "Mushroom"
        case DECORATION_MUSHROOM_190:  // dec91 "Mushroom"
            return 37;

        case DECORATION_FOSSIL_188:
        case DECORATION_FOSSIL_189:
        case DECORATION_STALAGMITE_191:
        case DECORATION_MUSHROOM_192:
        case DECORATION_MUSHROOM_193:
        case DECORATION_MUSHROOM_194:
        case DECORATION_TREE_195:
        case DECORATION_TREE_196:
        case DECORATION_TREE_197:
        case DECORATION_TREE_198:
        case DECORATION_TREE_199:
        case DECORATION_TREE_200:
        case DECORATION_TREE_201:
        case DECORATION_STATUE_202:
        case DECORATION_STATUE_203:
        case DECORATION_STATUE_204:
        case DECORATION_TREE_205:
            return 0;

        case DECORATION_BEACON_FIRE_206:
            return 162 + grng->random(7);  // dec60
        case DECORATION_BEACON_FIRE_207:
            return 169 + grng->random(7);  // dec61
        case DECORATION_BEACON_FIRE_208:
            return 176 + grng->random(7);  // dec62
        case DECORATION_BEACON_FIRE_209:
            return 183 + grng->random(7);  // dec63
        case DECORATION_MAGIC_PEDASTAL_210:
            return 150;  // dec64 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_211:
            return 151;  // dec65 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_212:
            return 152;  // dec66 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_213:
            return 153;  // dec67 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_214:
            return 154;  // dec68 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_215:
            return 155;  // dec69 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_216:
            return 156;  // dec70 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_217:
            return 157;  // dec71 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_218:
            return 158;  // dec72 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_219:
            return 159;  // dec73 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_220:
            return 160;  // dec74 "Magic pedestal"
        case DECORATION_MAGIC_PEDASTAL_221:
            return 161;  // dec75 "Magic pedestal"

        case DECORATION_BURNED_OUT_FIRE_222:
        case DECORATION_223:
        case DECORATION_TREE_224:
        case DECORATION_TREE_225:
        case DECORATION_TREE_226:
        case DECORATION_TREE_227:
            return 0;

        default:
            logger->error("Invalid DecorationDescID: {}", std::to_underlying(uDecorationDescID));
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
    case DECORATION_TRASH_HEAP_4:    // trash pile
    case DECORATION_CAMPFIRE_5:    // campfire
    case DECORATION_CAULDRON_6:    // cauldron
    case DECORATION_FRUIT_BOWL_11:   // fruit plate
    case DECORATION_TRASH_HEAP_13:   // trash pile
    case DECORATION_DIRT_14:   // dirt
    case DECORATION_CASK_24:   // keg
    case DECORATION_BARREL_34:   // barrel
    case DECORATION_FIRE_184:  // fire
    case DECORATION_MUSHROOM_187:  // mushroom
    case DECORATION_MUSHROOM_190:  // mushroom
        return true;
    default:
        break;
    }

    if (uDecorationDescID >= DECORATION_BEACON_FIRE_206 &&
        uDecorationDescID <= DECORATION_BEACON_FIRE_209)  // lighthouse fire
        return true;
    if (uDecorationDescID >= DECORATION_MAGIC_PEDASTAL_210 && uDecorationDescID <= DECORATION_MAGIC_PEDASTAL_221)  // magic pedestal
        return true;

    return false;
}
