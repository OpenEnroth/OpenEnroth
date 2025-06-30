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
    helm.itemId = ITEM_FULL_HELM; // Size = 2x2.
    inventory.add(Pointi(2, 2), helm);

    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
            Pointi pos(x, y);
            EXPECT_EQ(inventory.canAdd(pos, Sizei(1, 1)), !Recti(Pointi(2, 2), Sizei(2, 2)).contains(pos));
        }
    }
}

GAME_TEST(Inventory, CrazyBigItem) {
    // Checking that we can add an item with size == inventory size.
    Inventory inventory(Sizei(2, 2), Inventory::MAX_ITEMS);

    Item helm;
    helm.itemId = ITEM_FULL_HELM; // Size = 2x2, fills whole inventory.
    inventory.add(Pointi(0, 0), helm);

    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            EXPECT_EQ(inventory.canAdd(Pointi(x, y), Sizei(1, 1)), false);
        }
    }
}

GAME_TEST(Inventory, StorageFull) {
    // Checking that we can't add items to an inventory that's full.
    Inventory inventory(Sizei(1, 1), 1);

    Item ring;
    ring.itemId = ITEM_BRASS_RING;

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

GAME_TEST(Inventory, Iteration) {
    // Checking that loops over inventory items with `Item &` compile.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);

    for (Item &item : inventory.items()) {
        EXPECT_FALSE(true);
    }

    for (Item &item : inventory.items(ITEM_ARTIFACT_ELFBANE)) {
        EXPECT_FALSE(true);
    }
}

GAME_TEST(Inventory, AddOrder) {
    // MM7 adds items to inventory by columns, so we check that our code does the same.
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);

    InventoryEntry ring0 = inventory.add(Item(ITEM_BRASS_RING));
    InventoryEntry ring1 = inventory.add(Item(ITEM_ANGELS_RING));
    InventoryEntry dagger = inventory.add(Item(ITEM_DAGGER));
    InventoryEntry sword = inventory.add(Item(ITEM_CRUDE_LONGSWORD));

    EXPECT_EQ(ring0.geometry(), Recti(0, 0, 1, 1));
    EXPECT_EQ(ring1.geometry(), Recti(0, 1, 1, 1));
    EXPECT_EQ(dagger.geometry(), Recti(0, 2, 1, 3));
    EXPECT_EQ(sword.geometry(), Recti(1, 0, 1, 5));
}

GAME_TEST(Inventory, EntryInvalidation) {
    // Checking that taking out an item from under an `InventoryEntry` invalidates the entry.
    Inventory inventory;
    InventoryEntry entry = inventory.add(Pointi(0, 0), Item(ITEM_BRASS_RING));

    Item item = inventory.take(entry);
    EXPECT_EQ(item.itemId, ITEM_BRASS_RING);

    EXPECT_FALSE(entry);
    EXPECT_EQ(entry.geometry(), Recti());
    EXPECT_EQ(entry.slot(), ITEM_SLOT_INVALID);
    EXPECT_EQ(entry.zone(), INVENTORY_ZONE_INVALID);
}
