#pragma once

#include <string>
#include <optional>

#include "Engine/Data/HouseEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/ItemEnumFunctions.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Time/Time.h"
#include "Engine/MapEnums.h"

#include "Utility/IndexedArray.h"

class Character;

struct ItemGen {  // 0x24
    static void PopulateSpecialBonusMap();
    static void PopulateRegularBonusMap();
    static void PopulateArtifactBonusMap();
    static void ClearItemBonusMaps();

    void GetItemBonusArtifact(const Character *owner, CharacterAttribute attrToGet, int *bonusSum) const;
    void GetItemBonusSpecialEnchantment(const Character *owner, CharacterAttribute attrToGet, int *additiveBonus, int *halfSkillBonus) const;

    inline void ResetEnchantAnimation() { flags &= ~ITEM_ENCHANT_ANIMATION_MASK; }
    inline bool ItemEnchanted() const {
        return flags & ITEM_ENCHANT_ANIMATION_MASK;
    }
    inline bool AuraEffectRed() const {
        return (flags & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_RED;
    }
    inline bool AuraEffectBlue() const {
        return (flags & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_BLUE;
    }
    inline bool AuraEffectGreen() const {
        return (flags & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_GREEN;
    }
    inline bool AuraEffectPurple() const {
        return (flags & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_PURPLE;
    }

    bool IsRegularEnchanmentForAttribute(CharacterAttribute attrToGet);

    inline bool IsBroken() const { return flags & ITEM_BROKEN; }
    inline void SetBroken() { flags |= ITEM_BROKEN; }
    inline bool IsIdentified() const { return flags & ITEM_IDENTIFIED; }
    inline void SetIdentified() { flags |= ITEM_IDENTIFIED; }
    inline bool IsStolen() const { return flags & ITEM_STOLEN; }
    inline void SetStolen() { flags |= ITEM_STOLEN; }

    bool GenerateArtifact();
    void generateGold(ItemTreasureLevel treasureLevel);
    int GetValue() const;
    std::string GetDisplayName();
    std::string GetIdentifiedName();
    void UpdateTempBonus(Time time);
    void Reset();
    int _439DF3_get_additional_damage(DamageType *a2, bool *vampiyr);

    ItemType GetItemEquipType() const;
    CharacterSkillType GetPlayerSkillType() const;
    const std::string& GetIconName() const;
    uint8_t GetDamageDice() const;
    uint8_t GetDamageRoll() const;
    uint8_t GetDamageMod() const;
    bool canSellRepairIdentifyAt(HouseId houseId);

    bool isGold() const {
        return GetItemEquipType() == ITEM_TYPE_GOLD;
    }
    bool isShield() const {
        return GetItemEquipType() == ITEM_TYPE_SHIELD;
    }
    bool isWand() const {
        return GetItemEquipType() == ITEM_TYPE_WAND;
    }
    bool isPotion() const {
        return GetItemEquipType() == ITEM_TYPE_POTION;
    }
    bool isBook() const {
        return GetItemEquipType() == ITEM_TYPE_BOOK;
    }
    bool isReagent() const {
        return GetItemEquipType() == ITEM_TYPE_REAGENT;
    }
    bool isSpellScroll() const {
        return GetItemEquipType() == ITEM_TYPE_SPELL_SCROLL;
    }
    bool isMessageScroll() const {
        return GetItemEquipType() == ITEM_TYPE_MESSAGE_SCROLL;
    }

    bool isMeleeWeapon() const {
        return GetItemEquipType() == ITEM_TYPE_SINGLE_HANDED || GetItemEquipType() == ITEM_TYPE_TWO_HANDED;
    }
    bool isWeapon() const {
        return ::isWeapon(GetItemEquipType());
    }
    bool isArmor() const {
        return ::isArmor(GetItemEquipType());
    }
    bool isPassiveEquipment() const {
        return ::isPassiveEquipment(GetItemEquipType());
    }

    ItemId itemId = ITEM_NULL;
    int potionPower = 0; // Only for potions.
    int goldAmount = 0; // Only for gold.

    // TODO(captainurist): introduce ATTRIBUTE_NULL?
    std::optional<CharacterAttribute> standardEnchantment; // Standard (attribute) enchantment, if any.
    int standardEnchantmentStrength = 0; // Attribute enchantment strength - bonus value for the attribute.
    ItemEnchantment specialEnchantment = ITEM_ENCHANTMENT_NULL; // Special named enchantment, if any.
    int numCharges = 0; // Number of wand charges, wand disappears when this gets down to 0.
    int maxCharges = 0; // Max charges in a wand. This is used when recharging.
    ItemFlags flags = 0; // Item flags.
    ItemSlot equippedSlot = ITEM_SLOT_INVALID; // For equipped items - where is it equipped.
    int lichJarCharacterIndex = -1; // Only for full lich jars. 0-based index of the character whose earthly remains are stored in it.
                                    // Or whatever it is that's in the lich jar.
    bool placedInChest = false; // OE addition, whether the item was placed in the chest inventory area. Some chests
                                // are generated with more items than chest space, and this flag is used to track it.
    Time enchantmentExpirationTime; // Enchantment expiration time, if this item is temporarily enchanted. Note that
                                    // both special and attribute enchantments can be temporary, but in MM7 we only have
                                    // special temporary enchantments.
};

std::string GetItemTextureFilename(ItemId item_id, int index, int shoulder);

Segment<ItemTreasureLevel> RemapTreasureLevel(ItemTreasureLevel itemTreasureLevel, MapTreasureLevel mapTreasureLevel);

extern ItemGen *ptr_50C9A4_ItemToEnchant;
