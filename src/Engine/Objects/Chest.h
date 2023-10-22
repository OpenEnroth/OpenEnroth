#pragma once

#include <vector>
#include <string>
#include <optional>

#include "Engine/Objects/Items.h"
#include "Engine/Pid.h"

#include "Utility/Geometry/Vec.h"
#include "Utility/Memory/Blob.h"

#include "ChestEnums.h"

struct ChestDesc {
    std::string sName;
    int uWidth = 0;
    int uHeight = 0;
    int uTextureID = 0;
};

class ChestDescList {
 public:
    std::vector<ChestDesc> vChests;
};

struct Chest {
    inline bool initialized() const { return flags & CHEST_ITEMS_PLACED;}
    inline void setInitialized(bool b) {
        if (b)
            flags |= CHEST_ITEMS_PLACED;
        else
            flags &= ~CHEST_ITEMS_PLACED;
    }
    inline bool trapped() const { return flags & CHEST_TRAPPED; }

    static bool open(int uChestID, Pid objectPid);
    static bool chestUI_WritePointedObjectStatusString();
    static void onChestLeftClick();
    static void grabItem(bool all = false);

    uint16_t uChestBitmapID = 0;
    ChestFlags flags;
    std::array<ItemGen, 140> igChestItems;
    std::array<int16_t, 140> pInventoryIndices = {{}};  // 0x13b4 why is this a short?

    // Chest position, OE addition. Recalculated on level load in UpdateChestPositions. It's used to display
    // trap explosions in the same place regardless of which chest face was clicked.
    std::optional<Vec3i> position;
};

void setChestFlag(int chestId, ChestFlag flag, bool value);
void GenerateItemsInChest();
void UpdateChestPositions();

extern ChestDescList *pChestList;
extern std::vector<Chest> vChests;
