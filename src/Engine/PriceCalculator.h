#pragma once

class Character;
struct Item;
struct HouseData;

// Class to calculate various prices and overall deal with them
class PriceCalculator {
 public:
    /**
     * Originally was a method of class `Character`.
     *
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Base item identify price (before applying merchant skill).
     * @offset 0x4B824B
     */
    static int baseItemIdentifyPrice(float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param realValue                 Item base value.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Base item repair price (before applying merchant skill).
     * @offset 0x4B8265
     */
    static int baseItemRepairPrice(int realValue, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param realValue                 Item base value.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Base item buying price (before applying merchant skill).
     * @offset 0x4B8233
     */
    static int baseItemBuyingPrice(int realValue, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param realValue                 Item base value.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Base item selling price (before applying merchant skill).
     * @offset 0x4B8213
     */
    static int baseItemSellingPrice(int realValue, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param player                    Character trying to repair item.
     * @param realValue                 Item base value.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Price that player has to pay for repairing an item.
     * @offset 0x4B81C3
     */
    static int itemRepairPriceForPlayer(const Character *player, int realValue, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param player                    Character trying to identify the item.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Price that player has to pay for identifying an item.
     * @offset 0x4B8179
     */
    static int itemIdentificationPriceForPlayer(const Character *player, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param player                    Character buying the item.
     * @param realValue                 Real item value.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Price that player has to pay for buying an item.
     * @offset 0x4B8142
     */
    static int itemBuyingPriceForPlayer(const Character *player, int realValue, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param player                    Character selling the item.
     * @param item                      Item being sold.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Price that player will get for selling an item.
     * @offset 0x4B8102
     */
    static int itemSellingPriceForPlayer(const Character *player, const Item &item, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param player                    Character being healed.
     * @param priceMultiplier           General shop price multiplier.
     * @returns                         Price that player has to pay for healing in a temple.
     * @offset 0x4B807C
     */
    static int templeHealingCostForPlayer(const Character *player, float priceMultiplier);

    /**
     * Originally was a method of class `Character`.
     *
     * @param player                    Character to calculate value for.
     * @returns                         Merchant modifier for the character, a number in range (-inf, 100].
     * @offset 0x4911F3
     */
    static int playerMerchant(const Character *player);

    /**
     * @param player                    Character to calculate discount for.
     * @param goldAmount                Full price before applying the merchant skill.
     * @returns                         Price after applying the merchant skill.
     */
    static int applyMerchantDiscount(const Character *player, int goldAmount);

    /**
     * @param player                    Character to calculate discount for.
     * @param goldAmount                Full price before applying the merchant skill.
     * @returns                         Price after applying the merchant skill.
     */
    static int applyMerchantDiscount(const Character *player, float goldAmount);

    /**
     * @param player                    Character to calculate cost for.
     * @param house                     House in which player tries to learn skill.
     * @returns                         Skill learning cost for character.
     */
    static int skillLearningCostForPlayer(const Character *player, const HouseData &house);

    /**
     * @param player                    Character to calculate cost for.
     * @param house                     Stable/boat building in which player travels.
     * @returns                         Transport (stable/boat) cost for character.
     */
    static int transportCostForPlayer(const Character *player, const HouseData &house);

    /**
     * @param player                    Character to calculate cost for.
     * @param house                     Tavern in which player tries to buy room.
     * @returns                         Tavern room cost for character.
     */
    static int tavernRoomCostForPlayer(const Character *player, const HouseData &house);

    /**
     * @param player                    Character to calculate cost for.
     * @param house                     Tavern in which player tries to buy food.
     * @returns                         Tavern food cost for character.
     */
    static int tavernFoodCostForPlayer(const Character *player, const HouseData &house);

    /**
     * @param player                    Character to calculate cost for.
     * @param house                     Training hall in which player tries to train.
     * @returns                         Training cost for character.
     */
    static int trainingCostForPlayer(const Character *player, const HouseData &house);
};
