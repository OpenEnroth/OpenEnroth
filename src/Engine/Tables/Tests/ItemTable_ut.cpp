#include <memory>
#include <string>
#include <utility>

#include "Testing/Unit/UnitTest.h"

#include "Engine/Tables/ItemTable.h"
#include "Utility/Memory/Blob.h"

// MM8's items.txt uses the exact same column layout as MM7's, but its item roster runs past MM7's
// `ITEM_799` ceiling - MM8-only food/quest items (e.g. "Ball of Dunduck" cheese) live at rows 800-802, which
// used to be an out-of-bounds write into `ItemTable::items` (an `IndexedArray` sized to `ITEM_LAST_VALID`).
// Separately, row 0 in the file is a blank placeholder (matching the `ITEM_NULL` sentinel, not a real
// item) that used to underflow the same array, in MM7 and MM8 alike.
// Row/column values below follow MM8's items.txt (in `data/EnglishT.lod`), with
// flavor text trimmed - only the structural columns are exercised here.

namespace {
// Builds a full-size (803-row) items.txt blob so that `LoadItems` is exercised across MM8's entire real item
// range, not just a couple of hand-picked rows. Every row is a blank placeholder unless overridden below -
// this mirrors the blank rows that pad out the real file (e.g. row 0).
Blob mm8ItemsFixture() {
    std::string text =
        "\"1-199 equipable items, 200- potions, 300- scrolls, 400-books, 500- artifacts, 600 quest items 700- message scrolls \"\r\n"
        "Item #\tPic File\tName\tValue\tEquip Stat\tSkill Group\tMod1\tMod2\tmaterial\tID/Rep/St\tNot identified name\tSprite Index\tVarA\tVarB\tEquip X\tEquip Y\tNotes\r\n";

    for (int i = 0; i <= 802; i++) {
        switch (i) {
        case 1:
            text += "1\titem001\tLongsword\t50\tWeapon\tSword\t3d3\t0\t8\t1\tLongsword\t1\t0\t0\t14\t131\t\"Test description.\"\r\n";
            break;
        case 687:
            text += "687\titem354\tSiertal-laced ore\t50\tOre\tMisc\t0\t1\t2\t0\tLump of Ore\t134\t0\t0\t0\t0\t\"Test description.\"\r\n";
            break;
        case 800:
            text += "800\titem323\tWheel of Frelandeau\t0\t0\t0\t0\t0\t0\t0\tWheel of Frelandeau\t129\t0\t0\t0\t0\t\"Test description.\"\r\n";
            break;
        case 801:
            text += "801\titem324\tLog of Eldenbrie\t0\t0\t0\t0\t0\t0\t0\tLog of Eldenbrie\t130\t0\t0\t0\t0\t\"Test description.\"\r\n";
            break;
        case 802:
            text += "802\titem325\tBall of Dunduck\t0\t0\t0\t0\t0\t0\t0\tBall of Dunduck\t131\t0\t0\t0\t0\t\"Test description.\"\r\n";
            break;
        case 0:
            // Row 0 is a blank placeholder in the real file (matches the `ITEM_NULL` sentinel, not a real item).
            text += "0\t\t0\t\t\t\t\t\t0\t0\t\t0\t0\t0\t0\t0\t\r\n";
            break;
        default:
            // Dummy but well-formed row - only row indices and the "no throw" outcome matter here.
            text += std::to_string(i) + "\t\t0\t0\t\t\t0\t0\t0\t0\t\t0\t0\t0\t0\t0\t\r\n";
            break;
        }
    }

    return Blob::fromString(std::move(text));
}
} // namespace

UNIT_TEST(ItemTable, LoadItemsMm8FullRange) {
    auto table = std::make_unique<ItemTable>();
    table->LoadItems(mm8ItemsFixture()); // Shouldn't throw - row 0 and rows > 799 used to be OOB writes.

    // MM8's item id range is 802 (1..802 inclusive), 3 rows past MM7's 799 ceiling.
    EXPECT_EQ(table->items.size(), 802u);

    EXPECT_EQ(table->items[ITEM_CRUDE_LONGSWORD].name, "Longsword");

    EXPECT_EQ(table->items[static_cast<ItemId>(687)].name, "Siertal-laced ore");
    EXPECT_EQ(table->items[static_cast<ItemId>(687)].type, ITEM_TYPE_MISC); // "Ore" isn't a recognized equip stat, same fallback as MM7's quest items.

    EXPECT_EQ(table->items[ITEM_MM8_800].name, "Wheel of Frelandeau");
    EXPECT_EQ(table->items[ITEM_MM8_801].name, "Log of Eldenbrie");
    EXPECT_EQ(table->items[ITEM_MM8_802].name, "Ball of Dunduck");
}

namespace {
// spcitems.txt (special enchantments) uses the exact same 17-column layout and 72-row range in
// MM8 as it does in MM7. But two
// of those 72 rows carry different names under MM8: ordinal 39 (`ITEM_ENCHANTMENT_DEMON_SLAYING`) is
// "Ogre Slaying" instead of MM7's "Demon Slaying", and ordinal 63 (`ITEM_ENCHANTMENT_ELF_SLAYING`) is
// "Elemental Slaying" instead of MM7's "Elf Slaying" - same weapon-only spawn chance (20/20/20 on
// W1/W2/Miss), same "double damage" effect, different intended target creatures. Values below follow
// MM8's spcitems.txt (in `data/EnglishT.lod`), with flavor text trimmed.
Blob mm8SpecialEnchantmentsFixture() {
    std::string text =
        "\"Special Bonuses by Group\"\r\n"
        "\r\n"
        "Bonus Stat\tName Add\tW1\tW2\tMiss\tArm\tShld\tHelm\tBelt\tCape\tGaunt\tBoot\tRing\tAmul\tValue\tLvl\tDescription\r\n"
        "\r\n";

    for (int i = 1; i <= 72; i++) {
        if (i == 39) {
            text += "\"Placeholder bonus one.\"\tOgre Slaying\t20\t20\t20\t0\t0\t0\t0\t0\t0\t0\t0\t0\tX 2\tA\t\"Test description.\"\r\n";
        } else if (i == 63) {
            text += "\"Placeholder bonus two.\"\tElemental Slaying\t20\t20\t20\t0\t0\t0\t0\t0\t0\t0\t0\t0\tx2\tA\t\"Test description.\"\r\n";
        } else {
            text += "\"Placeholder.\"\tof Placeholder" + std::to_string(i) + "\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\tA\t\"Test description.\"\r\n";
        }
    }

    return Blob::fromString(std::move(text));
}
} // namespace

UNIT_TEST(ItemTable, LoadSpecialEnchantmentsMm8) {
    auto table = std::make_unique<ItemTable>();
    table->LoadSpecialEnchantments(mm8SpecialEnchantmentsFixture());

    EXPECT_EQ(table->specialEnchantments.size(), 72u);

    const SpecialEnchantmentData &ogreSlaying = table->specialEnchantments[ITEM_ENCHANTMENT_DEMON_SLAYING];
    EXPECT_EQ(ogreSlaying.itemSuffixOrPrefix, "Ogre Slaying");
    EXPECT_EQ(ogreSlaying.chanceByItemType[ITEM_TYPE_SINGLE_HANDED], 20);
    EXPECT_EQ(ogreSlaying.chanceByItemType[ITEM_TYPE_ARMOUR], 0); // Weapon-only spawn, same as MM7's Demon Slaying.

    const SpecialEnchantmentData &elementalSlaying = table->specialEnchantments[ITEM_ENCHANTMENT_ELF_SLAYING];
    EXPECT_EQ(elementalSlaying.itemSuffixOrPrefix, "Elemental Slaying");
    EXPECT_EQ(elementalSlaying.chanceByItemType[ITEM_TYPE_SINGLE_HANDED], 20);

    // Everything else in the range is unaffected: same suffix text/shape MM7 would produce for this fixture.
    EXPECT_EQ(table->specialEnchantments[ITEM_ENCHANTMENT_OF_PROTECTION].itemSuffixOrPrefix, "of Placeholder1");
    EXPECT_EQ(table->specialEnchantments[ITEM_ENCHANTMENT_LAST_VALID].itemSuffixOrPrefix, "of Placeholder72");
}
