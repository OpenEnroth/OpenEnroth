#pragma once
#include "Engine/Objects/Player.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"

int PriceCalculator::getBaseIdentifyPrice(float priceMultiplier) {
  int baseCost = (int)(priceMultiplier * 50.0f);

  if (baseCost < 1) {  // min price
      baseCost = 1;
  }

  return baseCost;
}

int PriceCalculator::getBaseRepairPrice(int uRealValue, float priceMultiplier) {
  int baseCost = (int)(uRealValue / (6.0f - priceMultiplier));

  if (baseCost < 1) {  // min price
      baseCost = 1;
  }

  return baseCost;
}

int PriceCalculator::getBaseBuyingPrice(int uRealValue,
                                        float price_multiplier) {
  int baseCost = (int)(uRealValue * price_multiplier);

  if (baseCost < 1) {  // min price
      baseCost = 1;
  }

  return baseCost;
}

int PriceCalculator::getBaseSellingPrice(int uRealValue,
                                         float priceMultiplier) {
  int baseCost = (int)(uRealValue / (priceMultiplier + 2.0f));

  if (baseCost < 1) {  // min price
      baseCost = 1;
  }

  return baseCost;
}

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