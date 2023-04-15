#pragma once
#include "Engine/PriceCalculator.h"

#include "Engine/Objects/Player.h"
#include "Engine/Party.h"

/**
 * @offset 0x4B824B
 * @brief Calculates base item identify price (before factoring in merchant).
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getBaseIdentifyPrice(float priceMultiplier) {
  int baseCost = (int)(priceMultiplier * 50.0f);

  if (baseCost < 1) {  // min price
    baseCost = 1;
  }

  return baseCost;
}

/**
 * @offset 0x4B8265
 * @brief Calculates base item repair price (before factoring in merchant).
 * @param uRealValue Item base value.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getBaseRepairPrice(int uRealValue, float priceMultiplier) {
  int baseCost = (int)(uRealValue / (6.0f - priceMultiplier));

  if (baseCost < 1) {  // min price
    baseCost = 1;
  }

  return baseCost;
}

/**
 * @offset 0x4B8233
 * @brief Calculates base item buying price (before factoring in merchant).
 * @param uRealValue Item base value.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getBaseBuyingPrice(int uRealValue,
                                        float price_multiplier) {
  int baseCost = (int)(uRealValue * price_multiplier);

  if (baseCost < 1) {  // min price
    baseCost = 1;
  }

  return baseCost;
}

/**
 * @offset 0x4B8213
 * @brief Calculates base item selling price (before factoring in merchant).
 * @param uRealValue Item base value.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getBaseSellingPrice(int uRealValue,
                                         float priceMultiplier) {
  int baseCost = (int)(uRealValue / (priceMultiplier + 2.0f));

  if (baseCost < 1) {  // min price
    baseCost = 1;
  }

  return baseCost;
}

/**
 * @offset 0x4B81C3
 * @brief Calculates price that player has to pay for repairing an item.
 * @param player Player trying to repair item.
 * @param uRealValue Item base value.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getItemRepairPriceForPlayer(Player* player, int uRealValue,
                                                 float priceMultiplier) {
  int baseCost = (int)(uRealValue / (6.0f - priceMultiplier));
  int actualCost = baseCost * (100 - getPlayerMerchant(player)) / 100;

  if (actualCost < baseCost / 3) {  // min price
    actualCost = baseCost / 3;
  }

  if (actualCost > 1) {
    return actualCost;
  } else {
    return 1;
  }
}

/**
 * @offset 0x4B8179
 * @brief Calculates price that player has to pay for identifying an item.
 * @param player Player trying to identify the item.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getItemIdentificationPriceForPlayer(
    Player* player, float priceMultiplier) {
  int baseCost = (int)(priceMultiplier * 50.0f);
  int actualCost = baseCost * (100 - getPlayerMerchant(player)) / 100;

  if (actualCost < baseCost / 3) {  // minimum price
    actualCost = baseCost / 3;
  }

  if (actualCost > 1) {
    return actualCost;
  } else {
    return 1;
  }
}

/**
 * @offset 0x4B8142
 * @brief Calculates price that player has to pay for buying an item.
 * @param player Player buying the item.
 * @param uRealValue Real item value.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getItemBuyingPriceForPlayer(Player* player,
                                                 unsigned int uRealValue,
                                                 float priceMultiplier) {
  uint price = (uint)(((100 - getPlayerMerchant(player)) *
                       (uRealValue * priceMultiplier)) /
                      100);

  if (price < uRealValue) {  // price should always be at least item value
    price = uRealValue;
  }

  return price;
}

/**
 * @offset 0x4B8102
 * @brief Calculates price that player will get for selling an item.
 * @param player Player buying the item.
 * @param item Item being bought.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getItemSellingPriceForPlayer(Player* player, ItemGen item,
                                                  float priceMultiplier) {
  int uRealValue = item.GetValue();
  int result = static_cast<int>((uRealValue / (priceMultiplier + 2.0)) +
                                uRealValue * getPlayerMerchant(player) / 100.0);

  if (result > uRealValue) {
    result = uRealValue;
  }

  if (item.IsBroken() || result < 1) {
    result = 1;
  }

  return result;
}

/**
 * @offset 0x4B807C
 * @brief Calculates price that player has to pay for healing in a temple.
 * @param player Player being healed.
 * @param priceMultiplier General shop price multiplier.
 *
 * Note: originally method of Player class.
 */
int PriceCalculator::getTempleHealingCostForPlayer(Player* player,
                                                   float priceMultiplier) {
  Condition conditionIdx =
      player->GetMajorConditionIdx();  // get worse condition
  int conditionTimeMultiplier = 1;
  int baseConditionMultiplier =
      1;  // condition good unless otherwise, base price for health and mana

  if (conditionIdx >= Condition_Dead &&
      conditionIdx <= Condition_Eradicated) {  // dead, petri, erad - serious
    if (conditionIdx <= Condition_Petrified) {
      baseConditionMultiplier = 5;  // dead or petri
    } else {
      baseConditionMultiplier = 10;  // erad
    }

    conditionTimeMultiplier = player->GetConditionDaysPassed(conditionIdx);
  } else if (conditionIdx < Condition_Dead) {  // all other conditions
    // get worst afflicted time of any other condition
    int highestTimeMultiplier;
    for (int i = 0; i <= 13; i++) {
      highestTimeMultiplier =
          player->GetConditionDaysPassed(static_cast<Condition>(i));

      if (highestTimeMultiplier > conditionTimeMultiplier) {
        conditionTimeMultiplier = highestTimeMultiplier;
      }
    }
  }

  int result =
      (int)((double)conditionTimeMultiplier * (double)baseConditionMultiplier *
            priceMultiplier);  // calc heal price

  // handle min-max costs
  result = std::clamp(result, 1, 10000);

  return result;
}

/**
 * @offset 0x4911F3
 * @brief Gets merchant value for player. May depend also on other things than
 * actual skill value.
 * @param player Player to calculate value for.
 * 
 * Note: originally method of Player class.
 */
int PriceCalculator::getPlayerMerchant(Player* player) {
  PLAYER_SKILL_LEVEL level = player->GetActualSkillLevel(PLAYER_SKILL_MERCHANT);
  PLAYER_SKILL_MASTERY mastery =
      player->GetActualSkillMastery(PLAYER_SKILL_MERCHANT);
  int multiplier =
      player->GetMultiplierForSkillLevel(PLAYER_SKILL_MERCHANT, 1, 2, 3, 5);

  if (mastery == PLAYER_SKILL_MASTERY_GRANDMASTER) {  // gm merchant
    return 10000;
  }

  int rep = pParty->GetPartyReputation();
  int bonus = multiplier * level;

  if (bonus == 0) {  // no skill so trading on rep alone
    return -rep;
  }

  return bonus - rep + 7;
}
