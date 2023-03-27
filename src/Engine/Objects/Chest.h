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
    explicit ChestDesc(struct ChestDesc_mm7 *pChest);

    std::string sName;
    unsigned int uWidth;
    unsigned int uHeight;
    unsigned int uTextureID;
};

class ChestList {
 public:
    ChestList() {}

    void FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8);

 public:
    std::vector<ChestDesc> vChests;
};

/*   65 */
#pragma pack(push, 1)
struct Chest {  // 0x14cc
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
    static bool Open(int uChestID);
    static void ToggleFlag(int uChestID, CHEST_FLAG uFlag, bool bValue);
    static bool ChestUI_WritePointedObjectStatusString();
    static void OnChestLeftClick();
    static void GrabItem(bool all = false);

    uint16_t uChestBitmapID{};        // 0
    CHEST_FLAGS uFlags;                // 2
    struct ItemGen igChestItems[140];       // 4
    int16_t pInventoryIndices[140]{};  // 0x13b4 why is this a short?
};
#pragma pack(pop)

void RemoveItemAtChestIndex(int index);
void GenerateItemsInChest();

size_t ChestsSerialize(char *pData);

extern ChestList *pChestList;
extern std::vector<Chest> vChests;
