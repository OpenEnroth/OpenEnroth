
// Class to calculate various prices and overall deal with them
class PriceCalculator
{
    public:
    /**
      * @offset 0x4B824B
      * @brief Calculates base item identify price (before factoring in merchant)
      * @param priceMultiplier General shop price multiplier.
      *
      * Note: originally method of Player class.
      */
    static int getBaseIdentifyPrice(float price_multiplier);

    /**
     * @offset 0x4B8265
     * @brief Calculates base item repair price (before factoring in merchant)
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int getBaseRepairPrice(int uRealValue, float price_multiplier);

    /**
     * @offset 0x4B8233
     * @brief Calculates base item buying price (before factoring in merchant)
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int getBaseBuyingPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4B8213
     * @brief Calculates base item selling price (before factoring in merchant)
     * @param uRealValue Item base value.
     * @param priceMultiplier General shop price multiplier.
     *
     * Note: originally method of Player class.
     */
    static int getBaseSellingPrice(int uRealValue, float priceMultiplier);

    /**
     * @offset 0x4911F3
     * @brief Gets merchant value for player. May depend also on other things than actual skill value.
     * @param player Player to calculate value for.
     *
     * Note: originally method of Player class.
     */
    static int getPlayerMerchant(Player* player);
};