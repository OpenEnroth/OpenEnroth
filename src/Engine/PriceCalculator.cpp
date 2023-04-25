#include <algorithm>

#include "Engine/PriceCalculator.h"

#include "Engine/Events2D.h"
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
    int actualCost = applyMerchantDiscount(player, baseCost);

    if (actualCost < baseCost / 3) {  // min price
        actualCost = baseCost / 3;
    }

    return std::max(1, actualCost);
}

int PriceCalculator::itemIdentificationPriceForPlayer(const Player *player, float priceMultiplier) {
    int baseCost = (int)(priceMultiplier * 50.0f);
    int actualCost = applyMerchantDiscount(player, baseCost);

    if (actualCost < baseCost / 3) {  // minimum price
        actualCost = baseCost / 3;
    }

    return std::max(1, actualCost);
}

int PriceCalculator::itemBuyingPriceForPlayer(const Player *player, unsigned int uRealValue, float priceMultiplier) {
    int price = applyMerchantDiscount(player, uRealValue * priceMultiplier);

    if (price < uRealValue) {  // price should always be at least item value
        price = uRealValue;
    }

    return price;
}

int PriceCalculator::itemSellingPriceForPlayer(const Player *player, const ItemGen &item, float priceMultiplier) {
    int uRealValue = item.GetValue();
    int result = static_cast<int>((uRealValue / (priceMultiplier + 2.0)) + uRealValue * playerMerchant(player) / 100.0);

    // can't get less than 1 gold or more than item is actually worth
    result = std::clamp(result, 1, uRealValue);

    if (item.IsBroken()) {
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

    int result = ((double)conditionTimeMultiplier * baseConditionMultiplier * priceMultiplier);  // calc heal price

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

int PriceCalculator::applyMerchantDiscount(const Player *player, int goldAmount) {
    return goldAmount * (100 - playerMerchant(player)) / 100;
}

int PriceCalculator::applyMerchantDiscount(const Player *player, float goldAmount) {
    return goldAmount * (100 - playerMerchant(player)) / 100;
}

int PriceCalculator::skillLearningCostForPlayer(const Player *player, const _2devent &house) {
    bool isGuild = house.uType >= BuildingType_FireGuild && house.uType <= BuildingType_SelfGuild;
    // guilds use different multiplier for skill learning
    int baseTeachPrice = (isGuild ? house.fPriceMultiplier : house.flt_24) * 500.0;
    if (house.uType == BuildingType_MercenaryGuild) {
        baseTeachPrice = 250;
    }
    int effectivePrice = applyMerchantDiscount(player, baseTeachPrice);
    if (effectivePrice < baseTeachPrice / 3) {
        effectivePrice = baseTeachPrice / 3;
    }
    return effectivePrice;
}
int PriceCalculator::transportCostForPlayer(const Player *player, const _2devent &house) {
    // boats are 2 times pricier than stables
    int basePrice = house.uType == BuildingType_Stables ? 25 : 50;

    int price = applyMerchantDiscount(player, basePrice * house.fPriceMultiplier);
    if (price < basePrice / 3) {
        price = basePrice / 3;
    }
    return price;
}

int PriceCalculator::tavernRoomCostForPlayer(const Player *player, const _2devent &house) {
    float houseMult = house.fPriceMultiplier;

    int roomPrice = applyMerchantDiscount(player, houseMult * houseMult / 10), minRoomPrice = ((houseMult * houseMult) / 10) / 3;

    if (roomPrice < minRoomPrice) {
        roomPrice = minRoomPrice;
    }

    if (roomPrice <= 0) {
        roomPrice = 1;
    }

    return roomPrice;
}

int PriceCalculator::tavernFoodCostForPlayer(const Player *player, const _2devent &house) {
    float houseMult = house.fPriceMultiplier;

    int foodPrice = applyMerchantDiscount(player, static_cast<float>(pow(houseMult, 3) / 100)), minFoodPrice = pow(houseMult, 3) / 300;
    if (foodPrice < minFoodPrice) {
        foodPrice = minFoodPrice;
    }

    if (foodPrice <= 0) {
        foodPrice = 1;
    }
    return foodPrice;
}

int PriceCalculator::trainingCostForPlayer(const Player *player, const _2devent &house) {
    int trainPrice = 0;
    uint64_t expForNextLevel = 1000ull * player->uLevel * (player->uLevel + 1) / 2;
    if (player->experience >= expForNextLevel) { // can train
        int playerClassTier = player->classType % 4 + 1;
        if (playerClassTier == 4) {
            playerClassTier = 3;
        }
        int baseTrainPrice = player->uLevel * house.fPriceMultiplier * (double)playerClassTier;
        trainPrice = applyMerchantDiscount(player, baseTrainPrice);
        if (trainPrice < baseTrainPrice / 3) {
            trainPrice = baseTrainPrice / 3;
        }
    }
    return trainPrice;
}
