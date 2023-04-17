#include "Engine/PriceCalculator.h"

#include "Engine/Objects/Items.h"
#include "Engine/Objects/Player.h"
#include "Engine/Party.h"

int PriceCalculator::baseItemIdentifyPrice(float priceMultiplier) {
    int baseCost = (int)(priceMultiplier * 50.0f);

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::baseItemRepairPrice(int uRealValue, float priceMultiplier) {
    int baseCost = (int)(uRealValue / (6.0f - priceMultiplier));

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::baseItemBuyingPrice(int uRealValue, float priceMultiplier) {
    int baseCost = (int)(uRealValue * priceMultiplier);

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::baseItemSellingPrice(int uRealValue, float priceMultiplier) {
    int baseCost = (int)(uRealValue / (priceMultiplier + 2.0f));

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::itemRepairPriceForPlayer(const Player *player, int uRealValue, float priceMultiplier) {
    int baseCost = (int)(uRealValue / (6.0f - priceMultiplier));
    int actualCost = baseCost * (100 - playerMerchant(player)) / 100;

    if (actualCost < baseCost / 3) {  // min price
        actualCost = baseCost / 3;
    }

    if (actualCost > 1) {
        return actualCost;
    } else {
        return 1;
    }
}

int PriceCalculator::itemIdentificationPriceForPlayer(const Player *player, float priceMultiplier) {
    int baseCost = (int)(priceMultiplier * 50.0f);
    int actualCost = baseCost * (100 - playerMerchant(player)) / 100;

    if (actualCost < baseCost / 3) {  // minimum price
        actualCost = baseCost / 3;
    }

    if (actualCost > 1) {
        return actualCost;
    } else {
        return 1;
    }
}

int PriceCalculator::itemBuyingPriceForPlayer(const Player *player, unsigned int uRealValue, float priceMultiplier) {
    uint price = (uint)(((100 - playerMerchant(player)) * (uRealValue * priceMultiplier)) / 100);

    if (price < uRealValue) {  // price should always be at least item value
        price = uRealValue;
    }

    return price;
}

int PriceCalculator::itemSellingPriceForPlayer(const Player *player, const ItemGen &item, float priceMultiplier) {
    int uRealValue = item.GetValue();
    int result = static_cast<int>((uRealValue / (priceMultiplier + 2.0)) + uRealValue * playerMerchant(player) / 100.0);

    if (result > uRealValue) {
        result = uRealValue;
    }

    if (item.IsBroken() || result < 1) {
        result = 1;
    }

    return result;
}

int PriceCalculator::templeHealingCostForPlayer(const Player *player, float priceMultiplier) {
    Condition conditionIdx = player->GetMajorConditionIdx();  // get worst condition
    int conditionTimeMultiplier = 1;
    int baseConditionMultiplier = 1;  // condition good unless otherwise, base price for health and mana

    if (conditionIdx >= Condition_Dead && conditionIdx <= Condition_Eradicated) {  // dead, petri, erad - serious
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
            highestTimeMultiplier = player->GetConditionDaysPassed(static_cast<Condition>(i));

            if (highestTimeMultiplier > conditionTimeMultiplier) {
                conditionTimeMultiplier = highestTimeMultiplier;
            }
        }
    }

    int result = (int)((double)conditionTimeMultiplier * (double)baseConditionMultiplier * priceMultiplier);  // calc heal price

    // handle min-max costs
    result = std::clamp(result, 1, 10000);

    return result;
}

int PriceCalculator::playerMerchant(const Player *player) {
    PLAYER_SKILL_LEVEL level = player->GetActualSkillLevel(PLAYER_SKILL_MERCHANT);
    PLAYER_SKILL_MASTERY mastery = player->GetActualSkillMastery(PLAYER_SKILL_MERCHANT);
    int multiplier = player->GetMultiplierForSkillLevel(PLAYER_SKILL_MERCHANT, 1, 2, 3, 5);

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
