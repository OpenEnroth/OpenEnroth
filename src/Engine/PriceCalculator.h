#pragma once

struct Player;
struct ItemGen;
struct _2devent;

// Class to calculate various prices and overall deal with them
class PriceCalculator {
 public:
    /**
     * @offset 0x4B824B
     * @brief Calculates base item identify price (before factoring in merchant).
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int baseItemIdentifyPrice(float priceMultiplier);

    /**
     * @offset 0x4B8265
     * @brief Calculates base item repair price (before factoring in merchant).
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int baseItemRepairPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8233
     * @brief Calculates base item buying price (before factoring in merchant).
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int baseItemBuyingPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8213
     * @brief Calculates base item selling price (before factoring in merchant).
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int baseItemSellingPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B81C3
     * @brief Calculates price that player has to pay for repairing an item.
     * @param player Player trying to repair item.
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int itemRepairPriceForPlayer(const Player *player, int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8179
     * @brief Calculates price that player has to pay for identifying an item.
     * @param player Player trying to identify the item.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int itemIdentificationPriceForPlayer(const Player *player, float priceMultiplier);

    /**
     * @offset 0x4B8142
     * @brief Calculates price that player has to pay for buying an item.
     * @param player Player buying the item.
     * @param uRealValue Real item value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int itemBuyingPriceForPlayer(const Player *player, unsigned int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8102
     * @brief Calculates price that player will get for selling an item.
     * @param player Player buying the item.
     * @param item Item being bought.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int itemSellingPriceForPlayer(const Player *player, const ItemGen &item, float priceMultiplier);

    /**
     * @offset 0x4B807C
     * @brief Calculates price that player has to pay for healing in a temple.
     * @param player Player being healed.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int templeHealingCostForPlayer(const Player *player, float priceMultiplier);

    /**
     * @offset 0x4911F3
     * @brief Gets effective merchant value for player. May depend also on other things than actual skill value.
     * @param player Player to calculate value for.
     *
     * Note: originally method of Player class.
     */
    static int playerMerchant(const Player *player);

    /**
     * @brief Applies player's merchant discount for given gold value.
     * @param player Player to calculate discount for.
     * @param goldAmount Full price before merchant.
     */
    static int applyMerchantDiscount(const Player *player, int goldAmount);

    /**
     * @brief Applies player's merchant discount for given gold value
     * @param player Player to calculate discount for.
     * @param goldAmount Full price before merchant.
     */
    static int applyMerchantDiscount(const Player *player, float goldAmount);

    /**
     * @brief Calculates skill learning cost for player.
     * @param player Player to calculate cost for.
     * @param house House in which player tries to learn skill.
     */
    static int skillLearningCostForPlayer(const Player *player, const _2devent &house);

    /**
     * @brief Calculates transport (stable/boat) cost for player.
     * @param player Player to calculate cost for.
     * @param house Stable/boat building in which player travels.
     */
    static int transportCostForPlayer(const Player *player, const _2devent &house);

    /**
     * @brief Calculates tavern room cost for player.
     * @param player Player to calculate cost for.
     * @param house Tavern in which player tries to buy room.
     */
    static int tavernRoomCostForPlayer(const Player *player, const _2devent &house);

    /**
     * @brief Calculates tavern food cost for player.
     * @param player Player to calculate cost for.
     * @param house Tavern in which player tries to buy food.
     */
    static int tavernFoodCostForPlayer(const Player *player, const _2devent &house);

    /**
     * @brief Calculates training cost for player.
     * @param player Player to calculate cost for.
     * @param house Training hall in which player tries to train.
     */
    static int trainingCostForPlayer(const Player *player, const _2devent &house);
};
