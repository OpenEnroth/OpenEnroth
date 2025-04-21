#pragma once

#include <vector>
#include <optional>

#include "Engine/Objects/Item.h"
#include "Engine/Pid.h"

#include "Library/Geometry/Vec.h"

#include "ChestEnums.h"

struct Chest {
    inline bool Initialized() const {
        return flags & CHEST_ITEMS_PLACED;
    }
    inline void SetInitialized(bool b) {
        if (b)
            flags |= CHEST_ITEMS_PLACED;
        else
            flags &= ~CHEST_ITEMS_PLACED;
    }
    inline bool Trapped() const { return flags & CHEST_TRAPPED; }

    static bool CanPlaceItemAt(int test_cell_position, ItemId item_id, int uChestID);
    static int FindFreeItemSlot(int uChestID);
    static int PutItemInChest(int a1, Item *a2, int uChestID);
    static void PlaceItemAt(unsigned int put_cell_pos, unsigned int uItemIdx, int uChestID);
    static void PlaceItems(int uChestID);
    static bool open(int uChestID, Pid objectPid);
    static void toggleFlag(int uChestID, ChestFlag uFlag, bool bValue);
    static bool ChestUI_WritePointedObjectStatusString();
    static void OnChestLeftClick();
    static void GrabItem(bool all = false);

    uint16_t chestTypeId = 0;
    ChestFlags flags;
    std::array<Item, 140> items;
    std::array<int16_t, 140> inventoryMatrix = {{}};  // 0x13b4 why is this a short?

    // Chest position, OE addition. Recalculated on level load in UpdateChestPositions. It's used to display
    // trap explosions in the same place regardless of which chest face was clicked.
    std::optional<Vec3f> position;
};

void RemoveItemAtChestIndex(int index);
void GenerateItemsInChest();
void UpdateChestPositions();

extern std::vector<Chest> vChests;
