#pragma once

#include <string>

#include "GUI/GUIWindow.h"

#include "Engine/Events2D.h"

#include "UIHouseEnums.h"

bool HouseUI_CheckIfPlayerCanInteract();
void TrainingDialog(const char *s);
void JailDialog();
void MagicShopDialog();
void GuildDialog();
void MercenaryGuildDialog();
bool IsTravelAvailable(int a1);

/**
 * @brief                               New function.
 *
 * @param schedule_id                   Index to transport_schedule.
 *
 * @return                              Number of days travel by transport will take with hireling modifiers.
 */
int GetTravelTimeTransportDays(int schedule_id);

void TravelByTransport();
void TempleDialog();
void TownHallDialog();
void BankDialog();
void TavernDialog();
void PlayHouseSound(unsigned int uHouseID, HouseSoundID sound);  // idb
void WeaponShopDialog();
void AlchemistDialog();
void ArmorShopDialog();
void SimpleHouseDialog();
void OnSelectShopDialogueOption(DIALOGUE_TYPE option);
void PrepareHouse(HOUSE_ID house);  // idb

void createHouseUI(HOUSE_ID houseId);

/**
 * @offset 0x44622E
 */
bool enterHouse(HOUSE_ID uHouseID);
void BackToHouseMenu();

void InitializaDialogueOptions_Tavern(BuildingType type);  // idb
void InitializaDialogueOptions_Shops(BuildingType type);
void InitializaDialogueOptions(BuildingType type);
void InitializeBuildingResidents();

extern int uHouse_ExitPic;
extern int dword_591080;
extern BuildingType in_current_building_type;  // 00F8B198
extern DIALOGUE_TYPE dialog_menu_id;     // 00F8B19C

int HouseDialogPressCloseBtn();

extern class Image *_591428_endcap;

// Originally was a packed struct.
struct HouseAnimDescr {
    const char *video_name;
    int field_4;
    int house_npc_id;
    BuildingType uBuildingType; // Originally was 1 byte.
    uint8_t uRoomSoundId;
    uint16_t padding_e;
};

extern std::array<const HouseAnimDescr, 196> pAnimatedRooms;

class GUIWindow_House : public GUIWindow {
 public:
    GUIWindow_House(Pointi position, Sizei dimensions, HOUSE_ID houseId, const std::string &hint = std::string());
    virtual ~GUIWindow_House() {}

    virtual void Update();
    virtual void Release();

    HOUSE_ID houseId() const {
        return static_cast<HOUSE_ID>(wData.val); // TODO(captainurist): drop all direct accesses to wData.val.
    }
};
