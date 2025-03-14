#include "Testing/Game/GameTest.h"

#include "Engine/Objects/Inventory.h"

GAME_TEST(Inventory, Initialization) {
    Inventory inventory(Sizei(5, 5), 50);
    EXPECT_EQ(inventory.storageSize(), Sizei(5, 5));
    EXPECT_EQ(inventory.size(), 0);
    EXPECT_EQ(inventory.capacity(), 50);
}

GAME_TEST(Inventory, Storage) {
    Inventory inventory(Sizei(5, 5));

    Item sword;
    sword.itemId = ITEM_CRUDE_LONGSWORD;
    EXPECT_TRUE(inventory.canStoreItem(Pointi(0, 0), sword.inventorySize()));

    Item *storedSword = inventory.storeItem(Pointi(0, 0), sword);
    ASSERT_NE(storedSword, nullptr);
    EXPECT_EQ(storedSword->itemId, ITEM_CRUDE_LONGSWORD);
    EXPECT_EQ(inventory.size(), 1);
    EXPECT_EQ(inventory.location(storedSword), INVENTORY_STORED);
    EXPECT_EQ(inventory.slot(storedSword), ITEM_SLOT_INVALID);
    EXPECT_EQ(inventory.geometry(storedSword), Recti(Pointi(0, 0), sword.inventorySize()));
}

GAME_TEST(Inventory, StorageFull) {
    Inventory inventory(Sizei(1, 1), 1);

    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    Item *storedRing = inventory.storeItem(Pointi(0, 0), ring);
    EXPECT_EQ(storedRing->itemId, ITEM_BRASS_RING);

    EXPECT_FALSE(inventory.canStoreItem(Pointi(0, 0), Sizei(1, 1)));
    EXPECT_FALSE(inventory.findStorage(Sizei(1, 1)));
    EXPECT_EQ(inventory.size(), inventory.capacity());
    EXPECT_FALSE(inventory.canEquipItem(ITEM_SLOT_RING1));
    EXPECT_FALSE(inventory.canBuryItem());
}

GAME_TEST(Inventory, Equipment) {
    Inventory inventory(Sizei(5, 5));
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    EXPECT_TRUE(inventory.canEquipItem(ITEM_SLOT_RING1));
    Item *equippedRing = inventory.equipItem(ITEM_SLOT_RING1, ring);
    ASSERT_NE(equippedRing, nullptr);
    EXPECT_EQ(equippedRing->itemId, ITEM_BRASS_RING);
    EXPECT_EQ(inventory.size(), 1);
    EXPECT_EQ(inventory.location(equippedRing), INVENTORY_EQUIPPED);
    EXPECT_EQ(inventory.slot(equippedRing), ITEM_SLOT_RING1);
    EXPECT_TRUE(inventory.geometry(equippedRing).isEmpty());
}

GAME_TEST(Inventory, Burial) {
    Inventory inventory(Sizei(5, 5));
    Item ring;
    ring.itemId = ITEM_BRASS_RING;

    EXPECT_TRUE(inventory.canBuryItem());
    Item *buriedRing = inventory.buryItem(ring);
    ASSERT_NE(buriedRing, nullptr);
    EXPECT_EQ(buriedRing->itemId, ITEM_BRASS_RING);
    EXPECT_EQ(inventory.size(), 1);
    EXPECT_EQ(inventory.location(buriedRing), INVENTORY_BURIED);
    EXPECT_EQ(inventory.slot(buriedRing), ITEM_SLOT_INVALID);
    EXPECT_TRUE(inventory.geometry(buriedRing).isEmpty());
}

GAME_TEST(Inventory, TakeItem) {
    Inventory inventory(Sizei(5, 5));
    Item sword;
    sword.itemId = ITEM_CRUDE_LONGSWORD;

    Item *equippedSword = inventory.equipItem(ITEM_SLOT_MAIN_HAND, sword);
    Item returnedSword = inventory.takeItem(equippedSword);
    EXPECT_EQ(returnedSword.itemId, ITEM_CRUDE_LONGSWORD);
    EXPECT_EQ(inventory.size(), 0);
    EXPECT_EQ(inventory.equippedItem(ITEM_SLOT_MAIN_HAND), nullptr);
}
