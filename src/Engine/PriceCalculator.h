#pragma once

class Character;
struct ItemGen;
struct BuildingDesc;

// Class to calculate various prices and overall deal with them
class PriceCalculator {
 public:
    /**
     * @offset 0x4B824B
     * @brief Calculates base item identify price (before factoring in merchant).
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int baseItemIdentifyPrice(float priceMultiplier);

    /**
     * @offset 0x4B8265
     * @brief Calculates base item repair price (before factoring in merchant).
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int baseItemRepairPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8233
     * @brief Calculates base item buying price (before factoring in merchant).
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int baseItemBuyingPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8213
     * @brief Calculates base item selling price (before factoring in merchant).
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int baseItemSellingPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B81C3
     * @brief Calculates price that player has to pay for repairing an item.
     * @param player Character trying to repair item.
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int itemRepairPriceForPlayer(const Character *player, int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8179
     * @brief Calculates price that player has to pay for identifying an item.
     * @param player Character trying to identify the item.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int itemIdentificationPriceForPlayer(const Character *player, float priceMultiplier);

    /**
     * @offset 0x4B8142
     * @brief Calculates price that player has to pay for buying an item.
     * @param player Character buying the item.
     * @param uRealValue Real item value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int itemBuyingPriceForPlayer(const Character *player, int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8102
     * @brief Calculates price that player will get for selling an item.
     * @param player Character buying the item.
     * @param item Item being bought.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int itemSellingPriceForPlayer(const Character *player, const ItemGen &item, float priceMultiplier);

    /**
     * @offset 0x4B807C
     * @brief Calculates price that player has to pay for healing in a temple.
     * @param player Character being healed.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Character class.
     */
    static int templeHealingCostForPlayer(const Character *player, float priceMultiplier);

    /**
     * @offset 0x4911F3
     * @brief Gets effective merchant value for player. May depend also on other things than actual skill value.
     * @param player Character to calculate value for.
     *
     * Note: originally method of Character class.
     */
    static int playerMerchant(const Character *player);

    /**
     * @brief Applies player's merchant discount for given gold value.
     * @param player Character to calculate discount for.
     * @param goldAmount Full price before merchant.
     */
    static int applyMerchantDiscount(const Character *player, int goldAmount);

    /**
     * @brief Applies player's merchant discount for given gold value
     * @param player Character to calculate discount for.
     * @param goldAmount Full price before merchant.
     */
    static int applyMerchantDiscount(const Character *player, float goldAmount);

    /**
     * @brief Calculates skill learning cost for player.
     * @param player Character to calculate cost for.
     * @param house House in which player tries to learn skill.
     */
    static int skillLearningCostForPlayer(const Character *player, const BuildingDesc &house);

    /**
     * @brief Calculates transport (stable/boat) cost for player.
     * @param player Character to calculate cost for.
     * @param house Stable/boat building in which player travels.
     */
    static int transportCostForPlayer(const Character *player, const BuildingDesc &house);

    /**
     * @brief Calculates tavern room cost for player.
     * @param player Character to calculate cost for.
     * @param house Tavern in which player tries to buy room.
     */
    static int tavernRoomCostForPlayer(const Character *player, const BuildingDesc &house);

    /**
     * @brief Calculates tavern food cost for player.
     * @param player Character to calculate cost for.
     * @param house Tavern in which player tries to buy food.
     */
    static int tavernFoodCostForPlayer(const Character *player, const BuildingDesc &house);

    /**
     * @brief Calculates training cost for player.
     * @param player Character to calculate cost for.
     * @param house Training hall in which player tries to train.
     */
    static int trainingCostForPlayer(const Character *player, const BuildingDesc &house);
};
