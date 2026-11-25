#pragma once

#include <string>
#include <array>
#include <vector>

#include "Engine/Data/HouseEnums.h"
#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/MonsterEnums.h"
#include "Engine/MapEnums.h"

#include "Core/Time/Duration.h"

#include "Utility/IndexedArray.h"
#include "Utility/Flags.h"

class Blob;
class ResourceManager;

// TODO(Nik-RE-dev): It seems that two greet flags are used purely because it's modification is performed
//                   before greeting string is constructed. It is also ensures that NPC in multi-NPC houses
//                   always greet you with first line until you leave the house.
//                   Ideally there should be only one flag.
enum class NpcFlag : uint32_t {
    NPC_GREETED_FIRST = 0x01, // NPC has been greeted first time
    NPC_GREETED_SECOND = 0x02, // NPC has been greeted second time
    NPC_HIRED = 0x80 // NPC is hired
};
using enum NpcFlag;
MM_DECLARE_FLAGS(NpcFlags, NpcFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(NpcFlags)

struct NPCTopic {
    std::string pTopic;
    std::string pText;
};

struct NPCData {  // 4Ch
    inline bool Hired() { return flags & NPC_HIRED; }

    std::string name; // Actual NPC name as displayed in-game.
    unsigned int portraitId = 0; // Portrait texture is "npcXXX" in icons.lod.
    NpcFlags flags = 0;
    int fame = 0; // Fame requirement for NPC to talk to the party, unused.
    int rep = 0; // NPC's reputation, is it even used in the game?
    HouseId house = HOUSE_INVALID; // House where this NPC is in.
    NpcProfession profession = NPC_PROFESSION_NONE;
    int greetingIndex = 0; // Index into "npcgreet.txt" for this NPC's greeting.
    bool canJoin = false;
    int field_24 = 0;
    unsigned int dialogue_1_evt_id = 0;  // dialogue options that are defined by script
    unsigned int dialogue_2_evt_id = 0;  // = 0  == unused
    unsigned int dialogue_3_evt_id = 0;  // can also be idx in pNPCTopics
    unsigned int dialogue_4_evt_id = 0;  // and absolutely crazy stuff when it's in party hierlings (npc2)
    unsigned int dialogue_5_evt_id = 0;
    unsigned int dialogue_6_evt_id = 0;
    Sex sex = SEX_MALE;
    int hasUsedAbility = 0;
    int newsTopic = 0;
};

struct NPCSacrificeStatus {
    bool inProgress = false; // Dark sacrifice is in progress for this hired NPC?
    Duration elapsedTime; // Time elapsed since the spell was cast.
    Duration endTime; // Total time of the animation - NPC will be removed once this time is reached.
};

struct NPCProfession {
    unsigned int uHirePrice{};
    std::string pBenefits{};
    std::string pActionText{};
    std::string pJoinText{};
    std::string pDismissText{};
};

struct NPCProfessionChance {
    IndexedArray<int, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> chanceByProfession = {{}};
    int total = 0;
};

struct NPCGreeting {
    std::string pGreeting1;  // at first meet
    std::string pGreeting2;  // at latest meets
};

struct NPCStats {
    void Initialize(ResourceManager *resourceManager);
    void InitializeNPCNames(const Blob &npcNames);
    void InitializeNPCNamesMM8(const Blob &pcNames);
    void InitializeNPCProfs(const Blob &npcProfs);
    void InitializeNPCText(const Blob &npcText);
    void InitializeNPCTopics(const Blob &npcTopics);
    void InitializeNPCDist(const Blob &npcDist);
    void InitializeNPCData(const Blob &npcData);
    void InitializeNPCGreets(const Blob &npcGreets);
    void InitializeNPCGroups(const Blob &npcGroups);
    void InitializeNPCNews(const Blob &npcNews);
    void InitializeAdditionalNPCs(NPCData *pNPCDataBuff, MonsterId npc_uid,
                                  HouseId uLocation2D, MapId uMapId);
    /**
     * Returns a random NPC name of the given gender starting with the same letter as `firstLetter`. Backs the
     * `%13` placeholder in `BuildDialogueString` - NPC dialogue templates that address the player by a similar-
     * sounding (mispronounced) name, e.g. "O Ho! %13! Er, %13. I think. Whatever...".
     *
     * The picked name is cached per `firstLetter`, so within a session an NPC mispronounces the player's name
     * the same way every time, and consecutive `%13` substitutions in the same template are consistent.
     *
     * Note: `%13` is only used by MM6's `npcbtb.txt`. MM7 has no templates that invoke this function.
     *
     * @param firstLetter               First letter of the player's name (case-insensitive).
     * @param gender                    Player's gender. Determines which name pool to draw from.
     * @return                          A name from `pNPCNames[gender]` that starts with `firstLetter`, or any
     *                                  random name from that pool if no name with that letter exists.
     * @offset 0x00495366
     */
    const std::string &sub_495366_MispronounceName(char firstLetter, Sex gender);

    // Sized to fit MM8's npcdata.txt, which has real (non-filler) NPCs up to index 527. MM7's table has 501 slots.
    // This is also the npcdata.bin save-game format (`SaveGame::npcData` in SaveLoad.h, `SaveGame_MM7::npcData` in
    // CompositeSnapshots.h) - all three must stay in lockstep. An MM7 npcdata.bin (501 entries) fails to
    // deserialize against this size with a BinarySerializationError.
    // MM8's npcdata.txt lists indices up to 1000, but every row from 528-1000 has the literal name
    // "Boob" and every other field zeroed - a dev placeholder, not real NPC content. `InitializeNPCData` only
    // warns when dropping a row that isn't this placeholder.
    std::array<NPCData, 528> pOriginalNPCData; // NPC data as read from npcdata.txt.
    std::array<NPCData, 528> pNPCData; // NPC data used during the game.
    IndexedArray<std::vector<std::string>, SEX_FIRST, SEX_LAST> pNPCNames = {};
    IndexedArray<NPCProfession, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> pProfessions = {};
    std::array<NPCData, 100> pAdditionalNPC = {{}};
    std::array<std::string, 52> pCatchPhrases{};   // 15CA4h
    std::array<std::string, 528> pNPCUnicNames{};  // NPC names from npcdata.txt, 1-based like the data above, kept in sync with pOriginalNPCData's size above.
    // Spans `MAP_FIRST`..`MAP_LAST_MM8` (not the MM7-only `MAP_LAST`), like
    // `MapTable::pInfos` - `InitializeAdditionalNPCs` indexes this by the currently loaded map's `MapId`, which
    // is in the MM8 range for MM8 maps.
    IndexedArray<NPCProfessionChance, MAP_FIRST, MAP_LAST_MM8> pProfessionChance;
    int field_17884 = 0;
    int field_17888 = 0;
    std::array<NPCGreeting, 206> pNPCGreetings;
    std::array<uint16_t, 51> pOriginalGroups = {{}}; // NPC groups as read from npcgroup.txt.
    std::array<uint16_t, 51> pGroups = {{}}; // NPC groups used during the game.
    int uNewlNPCBufPos = 0;
    int uNumNewNPCs = 0;
    int field_17FC8 = 0;
    int uNumNPCProfessions = 0;

    static int dword_AE336C_LastMispronouncedNameFirstLetter;
    static int dword_AE3370_LastMispronouncedNameResult;
};

// MM8's npctext.txt has non-empty text rows up to index 942 (npctopic.txt only up to 750).
// It lists indices up to 1047, but rows 943-1047 are all empty placeholders (index
// column only, no text). Sized to 1048 so the loader never has to drop a row. Not part of the save-game
// snapshot format, unlike `pOriginalNPCData` above.
extern std::array<NPCTopic, 1048> pNPCTopics;
extern NPCStats *pNPCStats;
