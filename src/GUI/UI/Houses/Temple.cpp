#include "Temple.h"

#include <string>
#include <vector>

#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIGame.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/LocationInfo.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Media/Audio/AudioPlayer.h"

void GUIWindow_Temple::mainDialogue() {
    int price = PriceCalculator::templeHealingCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()].fPriceMultiplier);
    std::string healString = fmt::format("{} {} {}", localization->GetString(LSTR_HEAL), price, localization->GetString(LSTR_GOLD));
    std::vector<std::string> optionsText = {isPlayerHealableByTemple(pParty->activeCharacter()) ? healString : "",
                                            localization->GetString(LSTR_DONATE), localization->GetString(LSTR_LEARN_SKILLS)};

    drawOptions(optionsText, colorTable.PaleCanary);
}

void GUIWindow_Temple::healDialogue() {
    if (!isPlayerHealableByTemple(pParty->activeCharacter())) {
        return;
    }

    int price = PriceCalculator::templeHealingCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()].fPriceMultiplier);
    if (pParty->GetGold() < price) {
        engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
        playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }

    bool setZombie = false;
    if (houseId() == HOUSE_TEMPLE_DEYJA || houseId() == HOUSE_TEMPLE_PIT || houseId() == HOUSE_TEMPLE_MOUNT_NIGHON) {
        setZombie = pParty->activeCharacter().conditions.Has(CONDITION_ZOMBIE);
        if (!pParty->activeCharacter().conditions.Has(CONDITION_ZOMBIE)) {
            if (pParty->activeCharacter().conditions.HasAny({CONDITION_ERADICATED, CONDITION_PETRIFIED, CONDITION_DEAD})) {
                pParty->activeCharacter().uPrevFace = pParty->activeCharacter().uCurrentFace;
                pParty->activeCharacter().uPrevVoiceID = pParty->activeCharacter().uVoiceID;
                pParty->activeCharacter().uVoiceID = (pParty->activeCharacter().GetSexByVoice() != SEX_MALE) + 23;
                pParty->activeCharacter().uCurrentFace = (pParty->activeCharacter().GetSexByVoice() != SEX_MALE) + 23;
                GameUI_ReloadPlayerPortraits(pParty->activeCharacterIndex() - 1, (pParty->activeCharacter().GetSexByVoice() != SEX_MALE) + 23);
                setZombie = true;
            }
        }
    } else {
        if (pParty->activeCharacter().conditions.Has(CONDITION_ZOMBIE)) {
            pParty->activeCharacter().uCurrentFace = pParty->activeCharacter().uPrevFace;
            pParty->activeCharacter().uVoiceID = pParty->activeCharacter().uPrevVoiceID;
            GameUI_ReloadPlayerPortraits(pParty->activeCharacterIndex() - 1, pParty->activeCharacter().uPrevFace);
        }
    }

    pParty->activeCharacter().conditions.ResetAll();
    if (setZombie) {
        pParty->activeCharacter().conditions.Set(CONDITION_ZOMBIE, pParty->GetPlayingTime());
    }
    pParty->TakeGold(price);
    pParty->activeCharacter().health = pParty->activeCharacter().GetMaxHealth();
    pParty->activeCharacter().mana = pParty->activeCharacter().GetMaxMana();
    pAudioPlayer->playExclusiveSound(SOUND_heal);
    pParty->activeCharacter().playReaction(SPEECH_TEMPLE_HEAL);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Temple::donateDialogue() {
    int price = houseTable[houseId()].fPriceMultiplier;
    if (pParty->GetGold() >= price) {
        pParty->TakeGold(price);
        LocationInfo *ddm = &currentLocationInfo();

        if (ddm->reputation > -5) {
            ddm->reputation -= 1;
        }
        int day = pParty->uCurrentDayOfMonth % 7;
        int counter = _templeSpellCounter[pParty->activeCharacterIndex() - 1] % 7;
        if (counter == day) {
            if (ddm->reputation <= -5) {
                pushTempleSpell(SPELL_AIR_WIZARD_EYE);
            }
            if (ddm->reputation <= -10) {
                pushTempleSpell(SPELL_SPIRIT_PRESERVATION);
            }
            if (ddm->reputation <= -15) {
                pushTempleSpell(SPELL_BODY_PROTECTION_FROM_MAGIC);
            }
            if (ddm->reputation <= -20) {
                pushTempleSpell(SPELL_LIGHT_HOUR_OF_POWER);
            }
            if (ddm->reputation <= -25) {
                pushTempleSpell(SPELL_LIGHT_DAY_OF_PROTECTION);
            }
        }
        _templeSpellCounter[pParty->activeCharacterIndex() - 1]++;
        pParty->activeCharacter().playReaction(SPEECH_TEMPLE_DONATE);
        engine->_statusBar->setEvent(LSTR_THANK_YOU);
    } else {
        engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
    }
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

GUIWindow_Temple::GUIWindow_Temple(HouseId houseId) : GUIWindow_House(houseId) {
    _templeSpellCounter.resize(pParty->pCharacters.size());
    std::fill(_templeSpellCounter.begin(), _templeSpellCounter.end(), 0);
}

void GUIWindow_Temple::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_Temple::houseSpecificDialogue() {
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TEMPLE_HEAL:
        healDialogue();
        break;
      case DIALOGUE_TEMPLE_DONATE:
        donateDialogue();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue(colorTable.PaleCanary);
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DialogueId> GUIWindow_Temple::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_TEMPLE_HEAL, DIALOGUE_TEMPLE_DONATE, DIALOGUE_LEARN_SKILLS};
      case DIALOGUE_LEARN_SKILLS:
        return {DIALOGUE_LEARN_UNARMED, DIALOGUE_LEARN_DODGE, DIALOGUE_LEARN_MERCHANT};
      default:
        return {};
    }
}

void GUIWindow_Temple::updateDialogueOnEscape() {
    if (IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_LEARN_SKILLS;
        return;
    }
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}

void GUIWindow_Temple::playHouseGoodbyeSpeech() {
    playHouseSound(houseId(), HOUSE_SOUND_TEMPLE_GOODBYE);
}

bool GUIWindow_Temple::isPlayerHealableByTemple(const Character &player) const {
    if (player.health >= player.GetMaxHealth() && player.mana >= player.GetMaxMana() && player.GetMajorConditionIdx() == CONDITION_GOOD) {
        // fully healthy
        return false;
    } else if (player.GetMajorConditionIdx() == CONDITION_ZOMBIE) {
        // zombie cant be healed at these tmeples
        return houseId() != HOUSE_TEMPLE_DEYJA && houseId() != HOUSE_TEMPLE_PIT && houseId() != HOUSE_TEMPLE_MOUNT_NIGHON;
    }

    return true;
}
