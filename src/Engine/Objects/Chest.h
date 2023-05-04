#pragma once

#include <vector>
#include <string>

#include "Engine/Objects/Items.h"

#include "Utility/Memory/Blob.h"
#include "Utility/Flags.h"

enum class CHEST_FLAG : uint16_t {
    CHEST_TRAPPED = 0x1,
    CHEST_ITEMS_PLACED = 0x2,
    CHEST_OPENED = 0x4,
};
using enum CHEST_FLAG;
MM_DECLARE_FLAGS(CHEST_FLAGS, CHEST_FLAG)
MM_DECLARE_OPERATORS_FOR_FLAGS(CHEST_FLAGS)

struct ChestDesc {
    std::string sName;
    int uWidth = 0;
    int uHeight = 0;
    int uTextureID = 0;
};

class ChestList {
 public:
    ChestList() {}

    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);

 public:
    std::vector<ChestDesc> vChests;
};

struct Chest {
    inline bool Initialized() const {
        return uFlags & CHEST_ITEMS_PLACED;
    }
    inline void SetInitialized(bool b) {
        if (b)
            uFlags |= CHEST_ITEMS_PLACED;
        else
            uFlags &= ~CHEST_ITEMS_PLACED;
    }
    inline bool Trapped() const { return uFlags & CHEST_TRAPPED; }

    static bool CanPlaceItemAt(int test_cell_position, ITEM_TYPE item_id, int uChestID);
    static int CountChestItems(int uChestID);
    static int PutItemInChest(int a1, struct ItemGen *a2, int uChestID);
    static void PlaceItemAt(unsigned int put_cell_pos, unsigned int uItemIdx, int uChestID);
    static void PlaceItems(int uChestID);
    static bool open(int uChestID);
    static void toggleFlag(int uChestID, CHEST_FLAG uFlag, bool bValue);
    static bool ChestUI_WritePointedObjectStatusString();
    static void OnChestLeftClick();
    static void GrabItem(bool all = false);

    uint16_t uChestBitmapID = 0;
    CHEST_FLAGS uFlags;
    std::array<ItemGen, 140> igChestItems;
    std::array<int16_t, 140> pInventoryIndices = {{}};  // 0x13b4 why is this a short?
};

void RemoveItemAtChestIndex(int index);
void GenerateItemsInChest();

extern ChestList *pChestList;
extern std::vector<Chest> vChests;
