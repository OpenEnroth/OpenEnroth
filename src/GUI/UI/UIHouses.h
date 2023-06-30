#pragma once

#include <string>
#include <vector>

#include "Engine/Tables/BuildingTable.h"
#include "Engine/Tables/AwardTable.h"

#include "GUI/GUIWindow.h"
#include "Utility/IndexedArray.h"
#include "UIHouseEnums.h"

// Right hand side dialogue writing constants
constexpr int SIDE_TEXT_BOX_WIDTH = 143;
constexpr int SIDE_TEXT_BOX_POS_X = 483;
constexpr int SIDE_TEXT_BOX_POS_Z = 334;
constexpr int SIDE_TEXT_BOX_POS_Y = 113;
constexpr int SIDE_TEXT_BOX_BODY_TEXT_HEIGHT = 174;
constexpr int SIDE_TEXT_BOX_BODY_TEXT_OFFSET = 138;
constexpr int SIDE_TEXT_BOX_MAX_SPACING = 32;

void BackToHouseMenu();

/**
 * @offset 0x4BCACC
 */
void onSelectHouseDialogueOption(DIALOGUE_TYPE option);

/**
 * @offset 0x44606A
 */
void prepareHouse(HOUSE_ID house);

void createHouseUI(HOUSE_ID houseId);

/**
 * @offset 0x44622E
 */
bool enterHouse(HOUSE_ID uHouseID);

bool houseDialogPressEscape();

/**
 * @offset 0x4B1E92
 */
void playHouseSound(HOUSE_ID houseID, HouseSoundType type);

void ClickNPCTopic(DIALOGUE_TYPE topic);

/**
 * @offset 0x4B4224
 */
void updateNPCTopics(int npc);

/**
 * Type of NPC you can have dialogue with inside house.
 */
enum class HouseNpcType {
    HOUSE_PROPRIETOR, // default resident in non-simple houses (shop owner, temple priest etc.).
    HOUSE_NPC,        // regular NPC, have description in NPCs table, @npc field is points to it.
    HOUSE_TRANSITION  // transition point to different map, @targetMapID contains ID of target map.
};
using enum HouseNpcType;

struct HouseNpcDesc {
    HouseNpcType type;
    std::string label = "";
    GraphicsImage *icon = nullptr;
    GUIButton *button = nullptr;
    int targetMapID = 0;
    NPCData *npc = nullptr;
};

class GUIWindow_House : public GUIWindow {
 public:
    explicit GUIWindow_House(HOUSE_ID houseId);
    virtual ~GUIWindow_House() {}

    virtual void Update();
    virtual void Release();

    BuildingType buildingType() const {
        return buildingTable[houseId()].uType;
    }

    HOUSE_ID houseId() const {
        return static_cast<HOUSE_ID>(wData.val); // TODO(captainurist): drop all direct accesses to wData.val.
    }

    void houseDialogManager();
    void houseNPCDialogue();
    void initializeDialog();
    void learnSelectedSkill(CharacterSkillType skill);
    void reinitDialogueWindow();
    bool checkIfPlayerCanInteract();

    void drawOptions(std::vector<std::string> &optionsText, Color selectColor,
                     int topOptionShift = 0, bool denseSpacing = false);

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option);
    // TODO(Nik-RE-dev): add DIALOGUE_TYPE argument?
    virtual void houseSpecificDialogue();
    virtual std::vector<DIALOGUE_TYPE> listDialogueOptions(DIALOGUE_TYPE option);
    virtual DIALOGUE_TYPE getOptionOnEscape();
    virtual void houseScreenClick();
    virtual void playHouseGoodbyeSpeech();

 protected:
    void learnSkillsDialogue();

    int _savedButtonsNum{};
    bool _transactionPerformed = false;
};

// Originally was a packed struct.
struct HouseAnimDescr {
    const char *video_name;
    int field_4;
    int house_npc_id;
    BuildingType uBuildingType; // Originally was 1 byte.
    uint8_t uRoomSoundId;
    uint16_t padding_e;
};

extern BuildingType in_current_building_type;  // 00F8B198
extern DIALOGUE_TYPE dialog_menu_id;     // 00F8B19C

extern class GraphicsImage *_591428_endcap;

extern std::array<const HouseAnimDescr, 196> pAnimatedRooms;

extern IndexedArray<int, BUILDING_WEAPON_SHOP, BUILDING_DARK_GUILD> itemAmountInShop;

extern std::vector<HouseNpcDesc> houseNpcs;
extern int currentHouseNpc;
