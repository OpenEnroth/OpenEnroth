#include "GUI/UI/Houses/MercenaryGuild.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Media/Audio/AudioPlayer.h"

std::array<int16_t, 49> word_4F0754;

void GUIWindow_MercenaryGuild::houseSpecificDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    /**
     * archiving this code just in case
     * I believe it is 250 gold cost for mercenary guild from mm6 and 100 for all other skill-learning house types in mm6
     * but they aren't used in mm7, so I'm gonna assume 250 gold cost in price calculator
     *
     *  int v32 = (uint8_t)(((buildingTable[window_SpeakInHouse->houseId()].uType != BuildingType_MercenaryGuild) - 1) & 0x96) + 100;
     *  int v3 = (int64_t)((double)v32 * buildingTable[window_SpeakInHouse->houseId()].fPriceMultiplier);
     *  pPrice = v3 * (100 - PriceCalculator::playerMerchant(&pParty->activeCharacter())) / 100;
     *  if (pPrice < v3 / 3) pPrice = v3 / 3;
     */
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[window_SpeakInHouse->houseId()]);

    if (_currentDialogue == DIALOGUE_MAIN) {
        if (!pParty->activeCharacter()._achievedAwardsBits[word_4F0754[2 * std::to_underlying(window_SpeakInHouse->houseId())]]) {
            // 171 looks like Mercenary Stronghold message from NPCNews.txt in MM6
            int pTextHeight = assets->pFontArrus->CalcTextHeight(pNPCTopics[171].pText, dialog_window.uFrameWidth, 0);
            dialog_window.DrawTitleText(assets->pFontArrus.get(), 0, (212 - pTextHeight) / 2 + 101, colorTable.PaleCanary, pNPCTopics[171].pText, 3);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }
        learnSkillsDialogue(colorTable.PaleCanary);
        return;
    }

    if (checkIfPlayerCanInteract()) {
        assert(false);  // what type of house that even is?
        // pSkillAvailabilityPerClass[8 + v58->uClass][4 + v23]
        // or
        // skillMaxMasteryPerClass[v58->uClass][v23 - 36]
        // or
        // skillMaxMasteryPerClass[v58->uClass - 1][v23 +
        // 1]
        assert(false);  // whacky condition - fix
        short *v6 = nullptr;
        if (false) {
            // TODO(captainurist): #mm6 this is MM6 legacy, and this decompiled code doesn't look sane.
            //                     Reimplement properly once we get to MM6.
            // if ( !*(&byte_4ED94C[37 * v1->uClass / 3] + dword_F8B19C)
            //|| (v6 = (short *)(&pParty->activeCharacter()._stats[CHARACTER_ATTRIBUTE_INTELLIGENCE] + _currentDialogue),
            //    *(short *)v6))
            pAudioPlayer->playUISound(SOUND_error);
        } else {
            int  v27;
            if (pParty->GetGold() < pPrice) {
                engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                v27 = 4;
            } else {
                pParty->TakeGold(pPrice);
                *(short *)v6 = 1;
                v27 = 2;
            }
            playHouseSound(houseId(), HouseSoundType(v27));
        }
    }
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_MercenaryGuild::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
}
