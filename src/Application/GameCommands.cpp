#include "GameCommands.h"

#include <Application/GameConfig.h>

#include <Engine/Party.h>
#include <Engine/Engine.h>

#include <GUI/GUIWindow.h>

#include <Media/Audio/AudioPlayer.h>

#include <utility>
#include <algorithm>
#include <string>
#include <vector>

template<typename TFunc>
void addCommand(const char* commandName, TFunc&& func, std::vector<std::string> defaultValues = {}) {
    engine->commandManager->addFunction(commandName, std::forward<TFunc>(func), defaultValues);
}

void playAwardSoundsOnAllCharacters() {
    for (auto&& character : pParty->pCharacters) {
        character.PlayAwardSound_Anim();
    }
}

ExecuteResult toggleBooleanConfig(const std::string &name, GameConfig::Bool &boolConfig) {
    boolConfig.toggle();
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    return commandSuccess(name + " " + std::string(boolConfig.value() ? "enabled." : "disabled."));
}

std::string alignmentToString(PartyAlignment alignment) {
    switch (alignment) {
    case PartyAlignment::PartyAlignment_Neutral: return "Neutral";
    case PartyAlignment::PartyAlignment_Good: return "Good";
    case PartyAlignment::PartyAlignment_Evil: return "Evil";
    }
    return "None";
}

void GameCommands::addCommands() {
    addCommand("gold", [](int goldAmount) {
        pParty->SetGold(goldAmount);
        return commandSuccess("Set amount of gold to " + std::to_string(goldAmount));
    });

    addCommand("xp", [](int xp) {
        pParty->GivePartyExp(xp);
        playAwardSoundsOnAllCharacters();
        return commandSuccess("Added " + std::to_string(xp) + " experience points to the party.");
    });

    addCommand("sp", [](int skillpoints) {
        for (Character& character : pParty->pCharacters) {
            character.uSkillPoints += skillpoints;
        }
        playAwardSoundsOnAllCharacters();
        return commandSuccess("Added " + std::to_string(skillpoints) + " skillpoints to every character.");
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
        return commandSuccess("Added all available skills to every character.");
    });

    addCommand("food", [](int foodAmount) {
        pParty->SetFood(foodAmount);
        return commandSuccess("Set amount of food to " + std::to_string(foodAmount));
    });

    addCommand("align", [](std::string alignment) {
        if (alignment.empty()) {
            return commandSuccess("Current alignment: " + alignmentToString(pParty->alignment));
        } else {
            if (alignment == "evil") { pParty->alignment = PartyAlignment::PartyAlignment_Evil;
            } else if (alignment == "good") { pParty->alignment = PartyAlignment::PartyAlignment_Good;
            } else if (alignment == "neutral") { pParty->alignment = PartyAlignment::PartyAlignment_Neutral;
            } else if (alignment == "cycle") {
                auto alignmentIndex = static_cast<int>(pParty->alignment);
                if (++alignmentIndex > static_cast<int>(PartyAlignment::PartyAlignment_Evil)) {
                    alignmentIndex = 0;
                }
                pParty->alignment = static_cast<PartyAlignment>(alignmentIndex);
            } else {
                return commandFailure("Invalid command. Choose any of: evil, good, neutral");
            }
        }

        SetUserInterface(pParty->alignment);
        return commandSuccess("Alignment changed to: " + alignmentToString(pParty->alignment));
    }, { "" });

    addCommand("wizardeye", []() {
        return toggleBooleanConfig("Wizard Eye", engine->config->debug.WizardEye);
    });

    addCommand("allmagic", []() {
        return toggleBooleanConfig("All Magic", engine->config->debug.AllMagic);
    });

    addCommand("help", []() {
        std::string result = "All Commands:\n";
        const auto& commands = engine->commandManager->getCommands();
        int index = 0;
        for (auto&& commandEntryPair : commands) {
            result += "- " + commandEntryPair.first + (index < commands.size() - 1 ? "\n" : "");
            ++index;
        }
        return commandSuccess(result);
    });
}
