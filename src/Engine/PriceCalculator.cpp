#include <algorithm>

#include "Engine/PriceCalculator.h"

#include "Engine/Data/HouseData.h"
#include "Engine/Objects/Item.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Party.h"

int PriceCalculator::baseItemIdentifyPrice(float priceMultiplier) {
    int baseCost = priceMultiplier * 50;

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::baseItemRepairPrice(int realValue, float priceMultiplier) {
    int baseCost = realValue / (6.0f - priceMultiplier);

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::baseItemBuyingPrice(int realValue, float priceMultiplier) {
    int baseCost = realValue * priceMultiplier;

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::baseItemSellingPrice(int realValue, float priceMultiplier) {
    int baseCost = realValue / (priceMultiplier + 2.0f);

    if (baseCost < 1) {  // min price
        baseCost = 1;
    }

    return baseCost;
}

int PriceCalculator::itemRepairPriceForPlayer(const Character *player, int realValue, float priceMultiplier) {
    int baseCost = realValue / (6.0f - priceMultiplier);
    int actualCost = applyMerchantDiscount(player, baseCost);

    if (actualCost < baseCost / 3) {  // min price
        actualCost = baseCost / 3;
    }

    return std::max(1, actualCost);
}

int PriceCalculator::itemIdentificationPriceForPlayer(const Character *player, float priceMultiplier) {
    int baseCost = priceMultiplier * 50;
    int actualCost = applyMerchantDiscount(player, baseCost);

    if (actualCost < baseCost / 3) {  // minimum price
        actualCost = baseCost / 3;
    }

    return std::max(1, actualCost);
}

int PriceCalculator::itemBuyingPriceForPlayer(const Character *player, int realValue, float priceMultiplier) {
    int price = applyMerchantDiscount(player, realValue * priceMultiplier);

    if (price < realValue) {  // price should always be at least item value
        price = realValue;
    }

    return price;
}

int PriceCalculator::itemSellingPriceForPlayer(const Character *player, const Item &item, float priceMultiplier) {
    int realValue = item.GetValue();
    int result = static_cast<int>((realValue / (priceMultiplier + 2.0)) + realValue * playerMerchant(player) / 100.0);

    if (realValue) {
        // can't get less than 1 gold or more than item is actually worth
        result = std::clamp(result, 1, realValue);
    } else {
        // TODO(Nik-RE-dev): blaster price is 0 and we can sell it for 1 gold because of the code below, but this is probably not how it should work
        result = 1;
    }

    if (item.IsBroken()) {
        result = 1;
    }

    return result;
}

int PriceCalculator::templeHealingCostForPlayer(const Character *player, float priceMultiplier) {
    Condition conditionIdx = player->GetMajorConditionIdx();  // get worst condition
    int conditionTimeMultiplier = 1;
    int baseConditionMultiplier = 1;  // condition good unless otherwise, base price for health and mana

    if (conditionIdx >= CONDITION_DEAD && conditionIdx <= CONDITION_ERADICATED) {  // dead, petri, erad - serious
        if (conditionIdx <= CONDITION_PETRIFIED) {
            baseConditionMultiplier = 5;  // dead or petri
        } else {
            baseConditionMultiplier = 10;  // erad
        }

        conditionTimeMultiplier = player->GetConditionDaysPassed(conditionIdx);
    } else if (conditionIdx < CONDITION_DEAD) {  // all other conditions
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

int PriceCalculator::playerMerchant(const Character *player) {
    CombinedSkillValue merchantSkill = player->getActualSkillValue(CHARACTER_SKILL_MERCHANT);
    int multiplier = player->GetMultiplierForSkillLevel(CHARACTER_SKILL_MERCHANT, 1, 2, 3, 5);

    if (merchantSkill.mastery() == CHARACTER_SKILL_MASTERY_GRANDMASTER) {  // gm merchant
        return 100;
    }

    int rep = pParty->GetPartyReputation();
    int bonus = multiplier * merchantSkill.level();

    if (bonus == 0) {  // no skill so trading on rep alone
        return -rep;
    }

    return std::min(bonus - rep + 7, 100);
}

int PriceCalculator::applyMerchantDiscount(const Character *player, int goldAmount) {
    return goldAmount * (100 - playerMerchant(player)) / 100;
}

int PriceCalculator::applyMerchantDiscount(const Character *player, float goldAmount) {
    return goldAmount * (100 - playerMerchant(player)) / 100;
}

int PriceCalculator::skillLearningCostForPlayer(const Character *player, const HouseData &house) {
    bool isGuild = house.uType >= HOUSE_TYPE_FIRE_GUILD && house.uType <= HOUSE_TYPE_SELF_GUILD;
    // guilds use different multiplier for skill learning
    int baseTeachPrice = (isGuild ? house.fPriceMultiplier : house.flt_24) * 500.0;
    if (house.uType == HOUSE_TYPE_MERCENARY_GUILD) {
        baseTeachPrice = 250;
    }
    int effectivePrice = applyMerchantDiscount(player, baseTeachPrice);
    if (effectivePrice < baseTeachPrice / 3) {
        effectivePrice = baseTeachPrice / 3;
    }
    return effectivePrice;
}

int PriceCalculator::transportCostForPlayer(const Character *player, const HouseData &house) {
    // boats are 2 times pricier than stables
    int basePrice = house.uType == HOUSE_TYPE_STABLE ? 25 : 50;

    int price = applyMerchantDiscount(player, basePrice * house.fPriceMultiplier);
    if (price < basePrice / 3) {
        price = basePrice / 3;
    }
    return price;
}

int PriceCalculator::tavernRoomCostForPlayer(const Character *player, const HouseData &house) {
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

int PriceCalculator::tavernFoodCostForPlayer(const Character *player, const HouseData &house) {
    float houseMult = house.fPriceMultiplier;

    int foodPrice = applyMerchantDiscount(player, static_cast<float>(std::pow(houseMult, 3) / 100)), minFoodPrice = std::pow(houseMult, 3) / 300;
    if (foodPrice < minFoodPrice) {
        foodPrice = minFoodPrice;
    }

    if (foodPrice <= 0) {
        foodPrice = 1;
    }
    return foodPrice;
}

int PriceCalculator::trainingCostForPlayer(const Character *player, const HouseData &house) {
    int trainPrice = 0;
    uint64_t expForNextLevel = 1000ull * player->uLevel * (player->uLevel + 1) / 2;
    if (player->experience >= expForNextLevel) { // can train
        int playerClassTier = getClassTier(player->classType);
        int baseTrainPrice = player->uLevel * house.fPriceMultiplier * (double)playerClassTier;
        trainPrice = applyMerchantDiscount(player, baseTrainPrice);
        if (trainPrice < baseTrainPrice / 3) {
            trainPrice = baseTrainPrice / 3;
        }
    }
    return trainPrice;
}
