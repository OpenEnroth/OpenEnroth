#include "Engine/Objects/NPC.h"

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Tables/NPCTable.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UITransition.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

int pDialogueNPCCount;
std::vector<GraphicsImage *> pDialogueNPCPortraits;

bool CheckPortretAgainstSex(int portret_num, int sex);

// All conditions for alive character excluding zombie
static const Segment<Condition> standardConditionsExcludeDead = {CONDITION_CURSED, CONDITION_UNCONSCIOUS};

// All conditions including dead character ones, but still excluding zombie
static const Segment<Condition> standardConditionsIncludeDead = {CONDITION_CURSED, CONDITION_ERADICATED};

//----- (004459F9) --------------------------------------------------------
NPCData *GetNPCData(signed int npcid) {
    NPCData *result;

    if (npcid >= 0) {
        if (npcid < 5000) {
            if (npcid >= 501) {
                logger->warning("NPC id exceeds MAX_DATA!");
            }
            return &pNPCStats->pNewNPCData[npcid];  // - 1];
        }
        return &pNPCStats->pAdditionalNPC[npcid - 5000];
    }

    if (npcid >= 5000) return &pNPCStats->pAdditionalNPC[npcid - 5000];
    if (sDialogue_SpeakingActorNPC_ID >= 0) {
        result = 0;
    } else {
        FlatHirelings buf;
        buf.Prepare();

        result = buf.Get(abs(sDialogue_SpeakingActorNPC_ID) - 1);
    }
    return result;
}

//----- (00445B2C) --------------------------------------------------------
struct NPCData *GetNewNPCData(signed int npcid, int *npc_indx) {
    NPCData *result;

    if (npcid >= 0) {
        if (npcid < 5000) {
            if (npcid >= 501) {
                logger->warning("NPC id exceeds MAX_DATA!");
            }
            *npc_indx = npcid;
            return &pNPCStats->pNewNPCData[npcid];
        }
        *npc_indx = npcid - 5000;
        return &pNPCStats->pAdditionalNPC[npcid - 5000];
    }
    if (npcid >= 5000) {
        *npc_indx = npcid - 5000;
        return &pNPCStats->pAdditionalNPC[npcid - 5000];
    }
    if (sDialogue_SpeakingActorNPC_ID >= 0) {
        *npc_indx = 0;
        result = nullptr;
    } else {
        FlatHirelings buf;
        buf.Prepare();

        result = buf.Get(abs(sDialogue_SpeakingActorNPC_ID) - 1);
    }
    return result;
}

//----- (00476387) --------------------------------------------------------
bool PartyHasDragon() { return pNPCStats->pNewNPCData[57].Hired(); }

//----- (00476395) --------------------------------------------------------
// 0x26 Wizard eye at skill level 2
bool CheckHiredNPCSpeciality(NPCProf prof) {
    if (bNoNPCHiring == 1) return false;

    for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        if (pNPCStats->pNewNPCData[i].profession == prof &&
            (pNPCStats->pNewNPCData[i].uFlags & NPC_HIRED)) {
            return true;
        }
    }
    return pParty->pHirelings[0].profession == prof
        || pParty->pHirelings[1].profession == prof;
}

//----- (004B40E6) --------------------------------------------------------
void NPCHireableDialogPrepare() {
    signed int v0;  // ebx@1
    NPCData *v1;    // edi@1

    v0 = 0;
    v1 = HouseNPCData[pDialogueNPCCount + -(dword_591080 != 0)];  //- 1
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0,
        UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_CANCEL), {ui_exit_cancel_button_background}
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_HouseScreenClick, 0);
    if (!pNPCStats->pProfessions[v1->profession].pBenefits.empty()) {
        pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0,
            UIMSG_ClickNPCTopic, DIALOGUE_PROFESSION_DETAILS, InputAction::Invalid, localization->GetString(LSTR_MORE_INFORMATION)
        );
        v0 = 1;
    }
    pDialogueWindow->CreateButton({480, 30 * v0 + 160}, {140, 30}, 1, 0,
        UIMSG_ClickNPCTopic, DIALOGUE_HIRE_FIRE, InputAction::Invalid, localization->GetString(LSTR_HIRE));
    pDialogueWindow->_41D08F_set_keyboard_control_group(v0 + 1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

//----- (004B4224) --------------------------------------------------------
void _4B4224_UpdateNPCTopics(int _this) {
    int num_menu_buttons;  // ebx@1
    NPCData *v17;          // [sp+10h] [bp-4h]@4

    num_menu_buttons = 0;
    pDialogueNPCCount = (_this + 1);
    Sizei renDims = render->GetRenderDimensions();
    if (_this + 1 == pDialogueNPCPortraits.size() && uHouse_ExitPic) {
        pDialogueWindow->Release();
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, renDims, 0);
        transition_button_label = localization->FormatString(LSTR_FMT_ENTER_S, pMapStats->pInfos[uHouse_ExitPic].pName.c_str()
        );
        pBtn_ExitCancel = pDialogueWindow->CreateButton({566, 445}, {75, 33}, 1, 0,
            UIMSG_Escape, 0, InputAction::No, localization->GetString(LSTR_CANCEL), {ui_buttdesc2});
        pBtn_YES = pDialogueWindow->CreateButton({486, 445}, {75, 33}, 1, 0,
            UIMSG_BF, 1, InputAction::Yes, transition_button_label, {ui_buttyes2});
        pDialogueWindow->CreateButton({pNPCPortraits_x[0][0], pNPCPortraits_y[0][0]}, {63, 73}, 1, 0,
            UIMSG_BF, 1, InputAction::EventTrigger, transition_button_label);
        pDialogueWindow->CreateButton({8, 8}, {460, 344}, 1, 0,
            UIMSG_BF, 1, InputAction::Yes, transition_button_label
        );
    } else {
        v17 = HouseNPCData[_this + 1 - ((dword_591080 != 0) ? 1 : 0)];  //+ 1
        if (dialog_menu_id == DIALOGUE_OTHER) {
            pDialogueWindow->Release();
        } else {
            for (int i = 0; i < pDialogueNPCPortraits.size(); ++i)
                HouseNPCPortraitsButtonsList[i]->Release();
        }
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {renDims.w, 345}, 0);
        pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0,
            UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_END_CONVERSATION), {ui_exit_cancel_button_background}
        );
        pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_HouseScreenClick, 0);
        dialog_menu_id = DIALOGUE_MAIN;
        if (pDialogueNPCCount == 1 && dword_591080) {
            window_SpeakInHouse->initializeDialog();
        } else {
            if (v17->is_joinable) {
                num_menu_buttons = 1;
                pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, DIALOGUE_13_hiring_related);
            }

            #define AddScriptedDialogueLine(EVENT_ID, MSG_PARAM) \
                if (EVENT_ID) { \
                    if (num_menu_buttons < 4) { \
                        int res = npcDialogueEventProcessor(EVENT_ID); \
                        if (res == 1 || res == 2) \
                            pDialogueWindow->CreateButton({480, 160 + 30 * num_menu_buttons++}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, MSG_PARAM); \
                    } \
                }

            AddScriptedDialogueLine(v17->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1);
            AddScriptedDialogueLine(v17->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2);
            AddScriptedDialogueLine(v17->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3);
            AddScriptedDialogueLine(v17->dialogue_4_evt_id, DIALOGUE_SCRIPTED_LINE_4);
            AddScriptedDialogueLine(v17->dialogue_5_evt_id, DIALOGUE_SCRIPTED_LINE_5);
            AddScriptedDialogueLine(v17->dialogue_6_evt_id, DIALOGUE_SCRIPTED_LINE_6);

            pDialogueWindow->_41D08F_set_keyboard_control_group(num_menu_buttons, 1, 0, 2);
            // TODO(Nik-RE-dev): initial number of buttons used only in non-simple houses now
            //                   but this causes situation where dead character can talk to someone
            //dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
        }
    }
}

//----- (00445C8B) --------------------------------------------------------
int GetGreetType(signed int SpeakingNPC_ID) {
    if (SpeakingNPC_ID >= 0) {
        if (SpeakingNPC_ID < 5000) return 1;  // QuestNPC_greet
        return 2;                             // HiredNPC_greet
    }

    if (SpeakingNPC_ID >= 5000) return 2;

    int v3 = abs((int)sDialogue_SpeakingActorNPC_ID) - 1;

    FlatHirelings buf;
    buf.Prepare();

    return buf.IsFollower(v3) ? 1 : 2;
}

//----- (00445308) --------------------------------------------------------
const std::string &GetProfessionActionText(NPCProf prof) {
    switch (prof) {
    case Healer:
    case ExpertHealer:
    case MasterHealer:
    case Cook:
    case Chef:
    case WindMaster:
    case WaterMaster:
    case GateMaster:
    case Acolyte:
    case Piper:
    case FallenWizard:
        return pNPCStats->pProfessions[prof].pActionText;
    default:
        return pNPCTopics[407].pTopic;
    }
}

//----- (004BB756) --------------------------------------------------------
int UseNPCSkill(NPCProf profession, int id) {
    switch (profession) {
        case Healer: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
            }
        } break;

        case ExpertHealer: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();

                for (Condition condition : standardConditionsExcludeDead) {
                    player.conditions.Reset(condition);
                }
            }
        } break;

        case MasterHealer: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();

                for (Condition condition : standardConditionsIncludeDead) {
                    // Master healer heals all except Eradicated and zombie
                    if (condition != CONDITION_ERADICATED) {
                        player.conditions.Reset(condition);
                    }
                }
            }
        } break;

        case Cook: {
            // Was 13
            if (pParty->GetFood() >= 14) {
                return 1;
            }

            pParty->GiveFood(1);
        } break;

        case Chef: {
            // Was 13
            if (pParty->GetFood() >= 14) {
                return 1;
            }

            if (pParty->GetFood() == 13) {
                pParty->GiveFood(1);
            } else {
                pParty->GiveFood(2);
            }
        } break;

        case WindMaster: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                GameUI_SetStatusBar(LSTR_CANT_FLY_INDOORS);
                pAudioPlayer->playUISound(SOUND_fizzle);
            } else {
                // Spell power was changed to 0 because it does not have meaning for this buff
                pParty->pPartyBuffs[PARTY_BUFF_FLY]
                    .Apply(pParty->GetPlayingTime() + GameTime::FromHours(2), CHARACTER_SKILL_MASTERY_MASTER, 0, 0, 0);
                // Mark buff as GM because NPC buff does not drain mana
                pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff = true;
                pAudioPlayer->playSpellSound(SPELL_AIR_FLY, 0, false);
            }
        } break;

        case WaterMaster: {
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                .Apply(pParty->GetPlayingTime() + GameTime::FromHours(3), CHARACTER_SKILL_MASTERY_MASTER, 0, 0, 0);
            // Mark buff as GM because NPC buff does not drain mana
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff = true;
            pAudioPlayer->playSpellSound(SPELL_WATER_WATER_WALK, 0, false);
        } break;

        case GateMaster: {
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
            pNextFrameMessageQueue->AddGUIMessage(UIMSG_OnCastTownPortal, PID(OBJECT_Player, pParty->pCharacters.size() + id), 0);
        } break;

        case Acolyte:
            pushNPCSpell(SPELL_SPIRIT_BLESS);
            break;
        case Piper:
            pushNPCSpell(SPELL_SPIRIT_HEROISM);
            break;
        case FallenWizard:
            pushNPCSpell(SPELL_LIGHT_HOUR_OF_POWER);
            break;

        case Teacher:
        case Instructor:
        case Armsmaster:
        case Weaponsmaster:
        case Apprentice:
        case Mystic:
        case Spellmaster:
        case Trader:
        case Merchant:
        case Scout:
        case Herbalist:
        case Apothecary:
        case Tinker:
        case Locksmith:
        case Fool:
        case ChimneySweep:
        case Porter:
        case QuarterMaster:
        case Factor:
        case Banker:
        case Horseman:
        case Bard:
        case Enchanter:
        case Cartographer:
        case Explorer:
        case Pirate:
        case Squire:
        case Psychic:
        case Gypsy:
        case Diplomat:
        case Duper:
        case Burglar:
        case Acolyte2:
        case Initiate:
        case Prelate:
        case Monk:
        case Sage:
        case Hunter:
            break;

        default:
            assert(false && "Invalid enum value");
    }
    return 0;
}

void FlatHirelings::Prepare() {
    count = 0;

    for (size_t i = 0; i < 2; ++i)
        if (!pParty->pHirelings[i].pName.empty())
            ids[count++] = i;

    for (size_t i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        NPCData *npc = &pNPCStats->pNewNPCData[i];
        if (npc->Hired()) {
            Assert(!npc->pName.empty()); // Important for the checks below.

            if (npc->pName != pParty->pHirelings[0].pName && npc->pName != pParty->pHirelings[1].pName) {
                Assert(i + 2 < 256); // Won't fit into uint8_t otherwise.
                ids[count++] = i + 2;
            }
        }
    }
}

bool FlatHirelings::IsFollower(size_t index) const {
    Assert(index < count);

    return ids[index] >= 2;
}

NPCData *FlatHirelings::Get(size_t index) const {
    Assert(index < count);

    uint8_t id = ids[index];

    if (id < 2)
        return &pParty->pHirelings[id];
    else
        return &pNPCStats->pNewNPCData[id - 2];
}
