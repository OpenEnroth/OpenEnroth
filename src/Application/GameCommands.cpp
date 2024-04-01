#include "GameCommands.h"

#include <Engine/Party.h>
#include <Engine/Engine.h>

#include <utility>
#include <algorithm>

template<typename TFunc>
void addCommand(const char* commandName, TFunc&& func) {
    engine->commandManager->addFunction(commandName, std::forward<TFunc>(func));
}

void playAwardSoundsOnAllCharacters() {
    for (auto&& character : pParty->pCharacters) {
        character.PlayAwardSound_Anim();
    }
}

void GameCommands::addCommands() {
    addCommand("gold", [](int goldAmount) {
        pParty->SetGold(goldAmount);
        return "Set amount of gold to " + std::to_string(goldAmount);
    });

    addCommand("xp", [](int xp) {
        pParty->GivePartyExp(xp);
        playAwardSoundsOnAllCharacters();
        return "Added" + std::to_string(xp) + " experience points to the party.";
    });

    addCommand("sp", [](int skillpoints) {
        for (Character& character : pParty->pCharacters) {
            character.uSkillPoints += skillpoints;
        }
        playAwardSoundsOnAllCharacters();
        return "Added" + std::to_string(skillpoints) + " skillpoints to every character.";
    });

    addCommand("skills", []() {
        for (auto&& character : pParty->pCharacters) {
            for (CharacterSkillType skill : allSkills()) {
                // if class can learn this skill
                if (skillMaxMasteryPerClass[character.classType][skill] > CHARACTER_SKILL_MASTERY_NONE) {
                    if (character.getSkillValue(skill) == CombinedSkillValue::none()) {
                        character.setSkillValue(skill, CombinedSkillValue::novice());
                    }
                }
            }
        }
        return "Added all available skills to every character.";
    });

    addCommand("food", [](int foodAmount) {
        pParty->SetFood(foodAmount);
        return "Set amount of food to " + std::to_string(foodAmount);
    });
}
