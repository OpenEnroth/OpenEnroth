#include "Testing/Game/GameTest.h"

#include "Engine/Objects/Inventory.h"

GAME_TEST(Inventory, Initialization) {
    // Sanity checks for Inventory construction.
    Inventory inventory(Sizei(5, 5), 50);
    EXPECT_EQ(inventory.gridSize(), Sizei(5, 5));
    EXPECT_EQ(inventory.size(), 0);
    EXPECT_EQ(inventory.capacity(), 50);
}

GAME_TEST(Inventory, Storage) {
    // Checking that adding an item to grid works and provides expected results.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);

    Item sword;
    sword.itemId = ITEM_CRUDE_LONGSWORD;
    EXPECT_TRUE(inventory.canAdd(Pointi(0, 0), sword));

    InventoryEntry storedSword = inventory.add(Pointi(0, 0), sword);
    ASSERT_TRUE(storedSword);
    EXPECT_EQ(storedSword->itemId, ITEM_CRUDE_LONGSWORD);
    EXPECT_EQ(storedSword.zone(), INVENTORY_ZONE_GRID);
    EXPECT_EQ(storedSword.slot(), ITEM_SLOT_INVALID);
    EXPECT_EQ(storedSword.geometry(), Recti(Pointi(0, 0), sword.inventorySize()));
    EXPECT_EQ(inventory.size(), 1);
}

GAME_TEST(Inventory, StorageCells) {
    // Checking that items properly occupy grid cells.
    Inventory inventory(Sizei(9, 9), Inventory::MAX_ITEMS);

    Item helm;
    helm.itemId = ITEM_FULL_HELM;
    inventory.add(Pointi(2, 2), helm);

    Recti helmRect(Pointi(2, 2), helm.inventorySize());
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
            Pointi pos(x, y);
            EXPECT_EQ(inventory.canAdd(pos, Sizei(1, 1)), !helmRect.contains(pos));
        }
    }
}

GAME_TEST(Inventory, CrazyBigItem) {
    // Checking that we can add an item with size == inventory size.
    Item helm;
    helm.itemId = ITEM_FULL_HELM;
    Sizei helmSize = helm.inventorySize();

    Inventory inventory(helmSize, Inventory::MAX_ITEMS);
    inventory.add(Pointi(0, 0), helm); // Fills the whole inventory.

    for (int x = 0; x < helmSize.w; x++)
        for (int y = 0; y < helmSize.h; y++)
            EXPECT_EQ(inventory.canAdd(Pointi(x, y), Sizei(1, 1)), false);
}

GAME_TEST(Inventory, StorageFull) {
    // Checking that we can't add items to an inventory that's full.
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    // Grid sized to exactly fit one ring, so that adding it fills the inventory to capacity.
    Inventory inventory(ring.inventorySize(), 1);

    InventoryEntry storedRing = inventory.add(Pointi(0, 0), ring);
    EXPECT_EQ(storedRing->itemId, ITEM_BRASS_RING);

    EXPECT_FALSE(inventory.canAdd(Pointi(0, 0), Sizei(1, 1)));
    EXPECT_FALSE(inventory.findSpace(Sizei(1, 1)));
    EXPECT_EQ(inventory.size(), inventory.capacity());
    EXPECT_FALSE(inventory.canEquip(ITEM_SLOT_RING1));
    EXPECT_FALSE(inventory.canStash());
}

GAME_TEST(Inventory, Equipment) {
    // Checking that equipping items works and provides expected results.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    EXPECT_TRUE(inventory.canEquip(ITEM_SLOT_RING1));
    InventoryEntry equippedRing = inventory.equip(ITEM_SLOT_RING1, ring);
    ASSERT_TRUE(equippedRing);
    EXPECT_EQ(equippedRing->itemId, ITEM_BRASS_RING);
    EXPECT_EQ(equippedRing.zone(), INVENTORY_ZONE_EQUIPMENT);
    EXPECT_EQ(equippedRing.slot(), ITEM_SLOT_RING1);
    EXPECT_TRUE(equippedRing.geometry().isEmpty());
    EXPECT_EQ(inventory.size(), 1);
}

GAME_TEST(Inventory, Stashing) {
    // Checking that stashing items works and provides expected results.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    EXPECT_TRUE(inventory.canStash());
    InventoryEntry buriedRing = inventory.stash(ring);
    ASSERT_TRUE(buriedRing);
    EXPECT_EQ(buriedRing->itemId, ITEM_BRASS_RING);
    EXPECT_EQ(buriedRing.zone(), INVENTORY_ZONE_STASH);
    EXPECT_EQ(buriedRing.slot(), ITEM_SLOT_INVALID);
    EXPECT_TRUE(buriedRing.geometry().isEmpty());
    EXPECT_EQ(inventory.size(), 1);
}

GAME_TEST(Inventory, TakeItem) {
    // Checking that taking an item from inventory works.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);
    Item sword;
    sword.itemId = ITEM_CRUDE_LONGSWORD;

    InventoryEntry equippedSword = inventory.equip(ITEM_SLOT_MAIN_HAND, sword);
    Item returnedSword = inventory.take(equippedSword);
    EXPECT_EQ(returnedSword.itemId, ITEM_CRUDE_LONGSWORD);
    EXPECT_EQ(inventory.size(), 0);
    EXPECT_FALSE(inventory.entry(ITEM_SLOT_MAIN_HAND));
}

GAME_TEST(Inventory, AddOrder) {
    // MM7 adds items to inventory by columns, so we check that our code does the same.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);

    Item ring0Item(ITEM_BRASS_RING);
    Item ring1Item(ITEM_ANGELS_RING);
    Item daggerItem(ITEM_DAGGER);
    Item swordItem(ITEM_CRUDE_LONGSWORD);

    InventoryEntry ring0 = inventory.add(ring0Item);
    InventoryEntry ring1 = inventory.add(ring1Item);
    InventoryEntry dagger = inventory.add(daggerItem);
    InventoryEntry sword = inventory.add(swordItem);

    // Column-first placement: ring0 goes into column 0, row 0. ring1 doesn't fit there anymore, so it goes
    // right below ring0, still in column 0. dagger doesn't fit column 0 (occupied) and, since ring0 is more
    // than 1 cell wide, doesn't fit column 1 either (blocked by ring0's overflow into that column) - so it
    // lands in column 2. sword then lands in column 3, the next free one.
    EXPECT_EQ(ring0.geometry(), Recti(Pointi(0, 0), ring0Item.inventorySize()));
    EXPECT_EQ(ring1.geometry(), Recti(Pointi(0, ring0Item.inventorySize().h), ring1Item.inventorySize()));
    EXPECT_EQ(dagger.geometry(), Recti(Pointi(2, 0), daggerItem.inventorySize()));
    EXPECT_EQ(sword.geometry(), Recti(Pointi(3, 0), swordItem.inventorySize()));
}

GAME_TEST(Inventory, EntryInvalidation) {
    // Checking that taking out an item from under an `InventoryEntry` invalidates the entry.
    Item ring(ITEM_BRASS_RING);
    Inventory inventory(ring.inventorySize(), Inventory::MAX_ITEMS);
    InventoryEntry entry = inventory.add(Pointi(0, 0), ring);

    Item item = inventory.take(entry);
    EXPECT_EQ(item.itemId, ITEM_BRASS_RING);

    EXPECT_FALSE(entry);
    EXPECT_EQ(entry.geometry(), Recti());
    EXPECT_EQ(entry.slot(), ITEM_SLOT_INVALID);
    EXPECT_EQ(entry.zone(), INVENTORY_ZONE_INVALID);
}
