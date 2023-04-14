#pragma once
#include "Engine/Objects/Items.h"

class Player;
// Class to calculate various prices and overall deal with them
class PriceCalculator {
 public:
  static int getBaseIdentifyPrice(float price_multiplier);
  static int getBaseRepairPrice(int uRealValue, float price_multiplier);
  static int getBaseBuyingPrice(int uRealValue, float priceMultiplier);
  static int getBaseSellingPrice(int uRealValue, float priceMultiplier);

  static int getItemRepairPriceForPlayer(Player* player, int uRealValue,
                                         float priceMultiplier);
  static int getItemIdentificationPriceForPlayer(Player* player,
                                                 float priceMultiplier);
  static int getItemBuyingPriceForPlayer(Player* player,
                                         unsigned int uRealValue,
                                         float priceMultiplier);
  static int getItemSellingPriceForPlayer(Player* player, ItemGen item,
                                          float priceMultiplier);

  static int getTempleHealingCostForPlayer(Player* player,
                                           float priceMultiplier);

  static int getPlayerMerchant(Player* player);
};
