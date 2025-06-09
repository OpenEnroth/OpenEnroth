#include "Testing/Game/GameTest.h"

#include "Engine/Objects/Inventory.h"

GAME_TEST(Inventory, Initialization) {
    Inventory inventory(Sizei(5, 5), 50);
    EXPECT_EQ(inventory.gridSize(), Sizei(5, 5));
    EXPECT_EQ(inventory.size(), 0);
    EXPECT_EQ(inventory.capacity(), 50);
}

GAME_TEST(Inventory, Storage) {
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);

    Item sword;
    sword.itemId = ITEM_CRUDE_LONGSWORD;
    EXPECT_TRUE(inventory.canAdd(Pointi(0, 0), sword.inventorySize()));

    InventoryEntry storedSword = inventory.add(Pointi(0, 0), sword);
    ASSERT_TRUE(storedSword);
    EXPECT_EQ(storedSword.item().itemId, ITEM_CRUDE_LONGSWORD);
    EXPECT_EQ(storedSword.zone(), INVENTORY_ZONE_GRID);
    EXPECT_EQ(storedSword.slot(), ITEM_SLOT_INVALID);
    EXPECT_EQ(storedSword.geometry(), Recti(Pointi(0, 0), sword.inventorySize()));
    EXPECT_EQ(inventory.size(), 1);
}

GAME_TEST(Inventory, StorageCells) {
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
    Inventory inventory(Sizei(1, 1), 1);

    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    InventoryEntry storedRing = inventory.add(Pointi(0, 0), ring);
    EXPECT_EQ(storedRing.item().itemId, ITEM_BRASS_RING);

    EXPECT_FALSE(inventory.canAdd(Pointi(0, 0), Sizei(1, 1)));
    EXPECT_FALSE(inventory.findSpace(Sizei(1, 1)));
    EXPECT_EQ(inventory.size(), inventory.capacity());
    EXPECT_FALSE(inventory.canEquip(ITEM_SLOT_RING1));
    EXPECT_FALSE(inventory.canStash());
}

GAME_TEST(Inventory, Equipment) {
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    EXPECT_TRUE(inventory.canEquip(ITEM_SLOT_RING1));
    InventoryEntry equippedRing = inventory.equip(ITEM_SLOT_RING1, ring);
    ASSERT_TRUE(equippedRing);
    EXPECT_EQ(equippedRing.item().itemId, ITEM_BRASS_RING);
    EXPECT_EQ(equippedRing.zone(), INVENTORY_ZONE_EQUIPMENT);
    EXPECT_EQ(equippedRing.slot(), ITEM_SLOT_RING1);
    EXPECT_TRUE(equippedRing.geometry().isEmpty());
    EXPECT_EQ(inventory.size(), 1);
}

GAME_TEST(Inventory, Burial) {
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    EXPECT_TRUE(inventory.canStash());
    InventoryEntry buriedRing = inventory.stash(ring);
    ASSERT_TRUE(buriedRing);
    EXPECT_EQ(buriedRing.item().itemId, ITEM_BRASS_RING);
    EXPECT_EQ(buriedRing.zone(), INVENTORY_ZONE_STASH);
    EXPECT_EQ(buriedRing.slot(), ITEM_SLOT_INVALID);
    EXPECT_TRUE(buriedRing.geometry().isEmpty());
    EXPECT_EQ(inventory.size(), 1);
}

GAME_TEST(Inventory, TakeItem) {
    Inventory inventory(Sizei(5, 5), Inventory::MAX_ITEMS);
    Item sword;
    sword.itemId = ITEM_CRUDE_LONGSWORD;

    InventoryEntry equippedSword = inventory.equip(ITEM_SLOT_MAIN_HAND, sword);
    Item returnedSword = inventory.take(equippedSword);
    EXPECT_EQ(returnedSword.itemId, ITEM_CRUDE_LONGSWORD);
    EXPECT_EQ(inventory.size(), 0);
    EXPECT_FALSE(inventory.entry(ITEM_SLOT_MAIN_HAND));
}
