#include "Engine/Events.h"
#include "Engine/Engine.h"
#include "Engine/Events2D.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/MapsLongTimer.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/stru123.h"
#include "Engine/stru159.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UITransition.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Platform/OSWindow.h"


std::array<EventIndex, 4400> pSomeOtherEVT_Events;
signed int uSomeOtherEVT_NumEvents;
char *pSomeOtherEVT;
std::array<EventIndex, 4400> pSomeEVT_Events;
signed int uSomeEVT_NumEvents;
char *pSomeEVT;

unsigned int uGlobalEVT_NumEvents;
unsigned int uGlobalEVT_Size;
std::array<char, 46080> pGlobalEVT;
std::array<EventIndex, 4400> pGlobalEVT_Index;

std::array<unsigned int, 500> pLevelStrOffsets;
unsigned int uLevelStrNumStrings;
unsigned int uLevelStrFileSize;
signed int uLevelEVT_NumEvents;
unsigned int uLevelEVT_Size;
std::array<char, 9216> pLevelStr;
std::array<char, 9216> pLevelEVT;
std::array<EventIndex, 4400> pLevelEVT_Index;

_2devent p2DEvents[525];

unsigned int LoadEventsToBuffer(const char *pContainerName, char *pBuffer,
                                unsigned int uBufferSize) {
    size_t size = 0;
    void *ptr = pEvents_LOD->LoadCompressedTexture(pContainerName, &size);
    if ((ptr == nullptr) || (size > uBufferSize)) {
        Error("File %s Size %lu - Buffer size %lu", pContainerName, size, uBufferSize);
    }

    memcpy(pBuffer, ptr, size);
    free(ptr);
    return size;
}

//----- (00443DA1) --------------------------------------------------------
void Initialize_GlobalEVT() {
    struct raw_event_header {
        unsigned char evt_size;
        unsigned char evt_id_l;
        unsigned char evt_id_h;
        unsigned char evt_sequence_num;
    };
    uint events_count;
    unsigned int offset_in;
    raw_event_header *current_hdr;
    uGlobalEVT_NumEvents = 0;
    uGlobalEVT_Size =
        LoadEventsToBuffer("global.evt", pGlobalEVT.data(), 46080);
    if (!uGlobalEVT_Size) return;
    memset(pGlobalEVT_Index.data(), 0x80, sizeof(pGlobalEVT_Index));  // 52800
    events_count = uGlobalEVT_NumEvents;
    current_hdr = (raw_event_header *)pGlobalEVT.data();
    offset_in = 0;
    for (events_count = 0, offset_in = 0; offset_in < uGlobalEVT_Size;
         ++events_count) {
        pGlobalEVT_Index[events_count].uEventID =
            current_hdr->evt_id_l + (current_hdr->evt_id_h << 8);
        pGlobalEVT_Index[events_count].event_sequence_num =
            current_hdr->evt_sequence_num;
        pGlobalEVT_Index[events_count].uEventOffsetInEVT = offset_in;
        offset_in += current_hdr->evt_size + 1;

        current_hdr = (raw_event_header *)&pGlobalEVT[offset_in];
    }
    uGlobalEVT_NumEvents = events_count;

    assert(uGlobalEVT_NumEvents < 4400);
}

//----- (00443EF8) --------------------------------------------------------
void LoadLevel_InitializeLevelEvt() {
    struct raw_event_header {
        unsigned char evt_size;
        unsigned char evt_id_l;
        unsigned char evt_id_h;
        unsigned char evt_sequence_num;
    };
    uint events_count;
    unsigned int offset_in;
    raw_event_header *current_hdr;

    if (!uLevelEVT_Size) return;

    memset(MapsLongTimersList, 0, 3200);
    memset(pLevelEVT_Index.data(), 80, sizeof(EventIndex) * 4400);

    uLevelEVT_NumEvents = 0;
    MapsLongTimers_count = 0;

    current_hdr = (raw_event_header *)pLevelEVT.data();
    offset_in = 0;
    for (events_count = 0, offset_in = 0; offset_in < uLevelEVT_Size;
         ++events_count) {
        pLevelEVT_Index[events_count].uEventID =
            current_hdr->evt_id_l + (current_hdr->evt_id_h << 8);
        pLevelEVT_Index[events_count].event_sequence_num =
            current_hdr->evt_sequence_num;
        pLevelEVT_Index[events_count].uEventOffsetInEVT = offset_in;
        offset_in += current_hdr->evt_size + 1;

        current_hdr = (raw_event_header *)&pLevelEVT[offset_in];
    }
    uLevelEVT_NumEvents = events_count;

    /*
    EmeraldIsle::Variables:
    [0] ???
    [1] ???
    [2] Luck Fountain uses left
    [3] Gold Fountain used this week
    [4] Gold Fountain total uses


    Emerald Isle #110 // Fire Resistance fountain
    0 LocationName
    0 if (Player.FireResistance < 50)
      {
    1   Set(Player.FireResistance, 50)
    2   SetFooterString(22) // +50 Fire Resistance (temporarily)
    3   Add(Party.Autonotes, 2)
    4   goto return
      }
    5 SetFooterString(11) // Refreshing!
    6 return



    Emerald Isle #111 // ???
      Initialize
      Set(Map.Variables[0], 30)
      Set(Map.Variables[1], 30)


    Emerald Isle #114 // month timer - manage luck fountain
    0   LocationName
    0     if (Player.BaseLuck >= 15)
        {
    2     SetFooterString(11) // Refreshing!
    3     return
        }
        else
        {
    1     if (Map.Variables[2] >= 1)
          {
    4       Sub(Map.Variables[2], 1)
    5       Add(Player.BaseLuck, 2)
    6       SetFooterString(25) // +2 Luck (Permament)
    7       return
          }
          else
          {
    2       SetFooterString(11) // Refreshing!
    3       return
          }
        }

    8   Initialize
    9   Set Map.Variables[2], 8



    Emerald Isle #115 // week timer - gold fountain in the center of town
    0 LocationName
    0 if (Map.Variables[4] < 3)
      {
    1   if (Map.Variables[3] == 0)
        {
    2     if (Party.Gold < 201)
          {
    3       if (Player.BaseLuck >= 15)
            {
    5         Add(Map.Variables[3], 1)
    6         Add(Party.Gold, 1000)
    7         Add(Map.Variables[4], 1)
    8         goto return
            }
            else
            {
    4         goto 9
            }
          }
        }
      }
    9 SetFooterString(11) // Refreshing!
    10  return

    11  Initialize
    12  Set(Map.Variables[3], 0)





    Emerald Isle #220 // day timer - monster spawner
    0 LocationName
    0 Initialize
    1 if (NumAliveActors(group=20) != 0)
    2   return
    3 SpawnMonsters(1, level=1, count=10, x=-336, y=14512, z=0,  group=20)
    4 SpawnMonsters(1, level=2, count=5,  x=16,   y=16352, z=90, group=20)
    5 SpawnMonsters(1, level=1, count=10, x=480,  y=18288, z=6,  group=20)



    Emerald Isle #200 // margareth dock tip
    0 if (!QBits.QuestDone[17])
      {
    1   InitiateNPCDialogue(npc=19)
      }
    2 return


    Emerald Isle #201 // margareth armoury tip
    0 if (!QBits.QuestDone[17])
      {
    1   InitiateNPCDialogue(npc=20)
      }
    2 return
    */
}

//----- (0044684A) --------------------------------------------------------
void EventProcessor(int uEventID, int targetObj, int canShowMessages,
                    int entry_line) {
    signed int v4;         // esi@7
    int v11;               // eax@14
    // char *v12;             // eax@15
    // const char *v16;       // esi@21
    // bool v17;              // edx@21
    // int v18;               // ecx@22
    int v19;               // ebp@36
    signed int v20;        // ecx@40
    int v21;               // eax@40
    int v22;               // edx@40
    int v23;               // eax@40
    unsigned __int16 v24;  // ax@45
    LevelDecoration *v26;  // eax@55
    int v27;               // eax@57
    int pEventID;          // eax@58
    int pNPC_ID;           // ecx@58
    int pIndex;            // esi@58
    NPCData *pNPC;         // ecx@58
    int v38;               // eax@78
    int v39;               // ecx@78
    int v42;               // eax@84
    int v43;               // ecx@84
    // GUIButton *v48;        // ecx@93
    // GUIButton *v49;        // esi@94
    signed int pValue;     // ebp@124
    Player *pPlayer;       // esi@125
    int v83;               // eax@212
    int v84;               // ebp@220
    int v90;               // eax@243
    const char *v91;       // ecx@247
    int v94;               // ecx@262
    int v95;               // ebp@262
    int v96;               // edx@262
    int v97;               // eax@262
    unsigned int v98;      // edx@265
    const char *v99;       // esi@267
    int v100;              // edx@267
    unsigned int v102;     // esi@281
    int v104;              // eax@288
    int v106;              // [sp-20h] [bp-4C8h]@278
    signed int v109;       // [sp-14h] [bp-4BCh]@278
    signed int v110;       // [sp-10h] [bp-4B8h]@278
    int curr_seq_num;      // [sp+10h] [bp-498h]@4
    int v126;              // [sp+1Ch] [bp-48Ch]@262
    int player_choose;     // [sp+20h] [bp-488h]@4
    // int v128;              // [sp+24h] [bp-484h]@21
    int v129;              // [sp+24h] [bp-484h]@262
    signed int v130;       // [sp+28h] [bp-480h]@0
    int v132;              // [sp+30h] [bp-478h]@262
    signed int v133;       // [sp+34h] [bp-474h]@1
    int v134;              // [sp+38h] [bp-470h]@262
    int v135;              // [sp+3Ch] [bp-46Ch]@262
    int v136;              // [sp+40h] [bp-468h]@40
    int v137;              // [sp+44h] [bp-464h]@40
    int v138;              // [sp+48h] [bp-460h]@40
    int v139;              // [sp+4Ch] [bp-45Ch]@40
    ItemGen item;          // [sp+50h] [bp-458h]@15
    // char Source[120];      // [sp+74h] [bp-434h]@15
    // char Str[120];         // [sp+ECh] [bp-3BCh]@21
    Actor Dst;             // [sp+164h] [bp-344h]@53

    v133 = 0;
    EvtTargetObj = targetObj;
    dword_5B65C4_cancelEventProcessing = 0;
    if (uEventID == 114) {  // for test script
        // if (!lua->DoFile("out01.lua"))
        //    logger->Warning("Error opening out01.lua\n");
        // logger->Warning("being tested that well\n");
        return;
    }
    if (!uEventID) {
        if (!game_ui_status_bar_event_string_time_left)
            GameUI_StatusBar_OnEvent(
                localization->GetString(521));  // Nothing here
        return;
    }
    player_choose = (uActiveCharacter == 0)
                        ? 6
                        : 4;  // 4 - active or  6 - random player if active =0
    curr_seq_num = entry_line;

    if (activeLevelDecoration) {
        uSomeEVT_NumEvents = uGlobalEVT_NumEvents;
        pSomeEVT = pGlobalEVT.data();
        memcpy(pSomeEVT_Events.data(), pGlobalEVT_Index.data(),
               sizeof(EventIndex) * 4400);  // 4400 evts
    } else {
        uSomeEVT_NumEvents = uLevelEVT_NumEvents;
        pSomeEVT = pLevelEVT.data();
        memcpy(pSomeEVT_Events.data(), pLevelEVT_Index.data(),
               sizeof(EventIndex) * 4400);
    }

    for (v4 = 0; v4 < uSomeEVT_NumEvents; ++v4) {
        if (dword_5B65C4_cancelEventProcessing) {
            if (v133 == 1) OnMapLeave();
            return;
        }
        if (pSomeEVT_Events[v4].uEventID == uEventID &&
            pSomeEVT_Events[v4].event_sequence_num == curr_seq_num) {
            _evt_raw *_evt =
                (_evt_raw *)(pSomeEVT + pSomeEVT_Events[v4].uEventOffsetInEVT);

            switch (_evt->_e_type) {
                case EVENT_CheckSeason:
                    if (!sub_4465DF_check_season(_evt->v5)) {
                        ++curr_seq_num;
                        // v4 = v124;
                        break;
                    }
                    v4 = -1;
                    curr_seq_num = _evt->v6 - 1;
                    ++curr_seq_num;
                    break;

                case EVENT_CheckSkill: {
                    v19 = _evt->v7 +
                          ((_evt->v8 +
                            ((_evt->v9 + ((unsigned int)_evt->v10 << 8)) << 8))
                           << 8);
                    if (player_choose < 0) goto LABEL_47;
                    if (player_choose <= 3) {
                        v24 =
                            pParty->pPlayers[0]
                                .pActiveSkills[3486 * player_choose + _evt->v5];
                    } else {
                        if (player_choose == 4) {
                            v24 = pPlayers[uActiveCharacter]
                                ->pActiveSkills[_evt->v5];
                        } else {
                            if (player_choose == 5) {
                                v20 = 0;
                                v21 = 3486 * v130 + _evt->v5;
                                v136 = 1;
                                HEXRAYS_LOWORD(v21) =
                                    pParty->pPlayers[0].pActiveSkills[v21];
                                v137 = v21 & 0x40;
                                v138 = v21 & 0x80;
                                v22 = v21 & 0x100;
                                v23 = v21 & 0x3F;
                                v139 = v22;
                                while (v23 < v19 || !*(&v136 + _evt->v6)) {
                                    ++v20;
                                    if (v20 >= 4) {
                                        ++curr_seq_num;
                                        break;
                                    }
                                }
                                curr_seq_num = _evt->v11 - 1;
                                ++curr_seq_num;
                                break;
                            }
LABEL_47:
                            // v10 = (ByteArray *)&v5[v9];
                            v24 = pParty->pPlayers[0]
                                      .pActiveSkills[_evt->v5 +
                                                     3486 * rand() % 4];
                        }
                    }
                    v136 = 1;
                    v137 = v24 & 0x40;
                    v138 = v24 & 0x80;
                    v139 = v24 & 0x100;
                    if ((v24 & 0x3F) >= v19 && *(&v136 + _evt->v6)) {
                        curr_seq_num = _evt->v11 - 1;
                        ++curr_seq_num;
                        break;
                    }
                    ++curr_seq_num;
                } break;

                case EVENT_SpeakNPC:
                    if (canShowMessages) {
                        // Actor::Actor(&Dst);
                        memset(&Dst, 0, 0x344u);
                        dword_5B65D0_dialogue_actor_npc_id =
                            _evt->v5 +
                            ((_evt->v6 +
                              ((_evt->v7 + ((unsigned int)_evt->v8 << 8)) << 8))
                             << 8);
                        Dst.sNPC_ID = dword_5B65D0_dialogue_actor_npc_id;
                        GameUI_InitializeDialogue(&Dst, false);
                    } else {
                        bDialogueUI_InitializeActor_NPC_ID =
                            _evt->v5 +
                            ((_evt->v6 +
                            ((_evt->v7 + ((unsigned int)_evt->v8 << 8)) << 8))
                                << 8);
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_ChangeEvent:
                    if (EVT_DWORD(_evt->v5)) {
                        stru_5E4C90_MapPersistVars._decor_events
                            [activeLevelDecoration->_idx_in_stru123] =
                            _evt->v5 - 124;
                    } else {
                        v26 = (LevelDecoration *)activeLevelDecoration;
                        stru_5E4C90_MapPersistVars._decor_events
                            [activeLevelDecoration->_idx_in_stru123] = 0;
                        v26->uFlags |= LEVEL_DECORATION_INVISIBLE;
                    }
                    ++curr_seq_num;

                    break;
                case EVENT_SetNPCGreeting:
                    v27 = EVT_DWORD(_evt->v5);
                    pNPCStats->pNewNPCData[v27].uFlags &= 0xFFFFFFFCu;
                    pNPCStats->pNewNPCData[v27].greet = EVT_DWORD(_evt->v9);
                    ++curr_seq_num;
                    break;
                case EVENT_SetNPCTopic: {
                    // v29 = _evt->v5 + ((_evt->v6 + ((_evt->v7 +
                    // ((uint)_evt->v8 << 8)) << 8)) << 8);
                    pEventID = _evt->v10 +
                               ((_evt->v11 +
                                 ((_evt->v12 + ((uint)_evt->v13 << 8)) << 8))
                                << 8);
                    pNPC_ID =
                        _evt->v5 +
                        ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                         << 8);
                    pIndex = _evt->v9;
                    pNPC = &pNPCStats->pNewNPCData[pNPC_ID];
                    if (pIndex == 0) pNPC->evt_A = pEventID;
                    if (pIndex == 1) pNPC->evt_B = pEventID;
                    if (pIndex == 2) pNPC->evt_C = pEventID;
                    if (pIndex == 3) pNPC->evt_D = pEventID;
                    if (pIndex == 4) pNPC->evt_E = pEventID;
                    if (pIndex == 5) pNPC->evt_F = pEventID;
                    if (pNPC_ID == 8) {
                        if (pEventID == 78) {
                            HouseDialogPressCloseBtn();
                            window_SpeakInHouse->Release();
                            pParty->uFlags &= ~PARTY_FLAGS_1_0002;
                            if (EnterHouse(HOUSE_DARK_GUILD_PIT)) {
                                pAudioPlayer->StopChannels(-1, -1);
                                window_SpeakInHouse = new GUIWindow_House(
                                    0, 0, window->GetWidth(),
                                    window->GetHeight(), (GUIButton *)170, "");
                                window_SpeakInHouse->CreateButton(
                                    61, 424, 31, 0, 2, 94,
                                    UIMSG_SelectCharacter, 1, GameKey::Digit1, "");
                                window_SpeakInHouse->CreateButton(
                                    177, 424, 31, 0, 2, 94,
                                    UIMSG_SelectCharacter, 2, GameKey::Digit2, "");
                                window_SpeakInHouse->CreateButton(
                                    292, 424, 31, 0, 2, 94,
                                    UIMSG_SelectCharacter, 3, GameKey::Digit3, "");
                                window_SpeakInHouse->CreateButton(
                                    407, 424, 31, 0, 2, 94,
                                    UIMSG_SelectCharacter, 4, GameKey::Digit4, "");
                                window_SpeakInHouse->CreateButton(
                                    0, 0, 0, 0, 1, 0, UIMSG_CycleCharacters, 0,
                                    GameKey::Tab, "");
                                current_npc_text = pNPCTopics[90].pText;
                            }
                        }
                    }
                    ++curr_seq_num;
                } break;
                case EVENT_NPCSetItem:
                    sub_448518_npc_set_item(EVT_DWORD(_evt->v5),
                                            EVT_DWORD(_evt->v9), _evt->v13);
                    ++curr_seq_num;
                    break;
                case EVENT_SetActorItem:
                    Actor::GiveItem(EVT_DWORD(_evt->v5), EVT_DWORD(_evt->v9),
                                    _evt->v13);
                    ++curr_seq_num;
                    break;
                case EVENT_SetNPCGroupNews:
                    pNPCStats->pGroups_copy
                        [_evt->v5 +
                         ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                          << 8)] = _evt->v9 + ((uint)_evt->v10 << 8);
                    ++curr_seq_num;
                    break;
                case EVENT_SetActorGroup:
                    __debugbreak();
                    *(&pActors[0].uGroup + 0x11000000 * _evt->v8 +
                      209 * (_evt->v5 +
                             ((_evt->v6 + ((uint)_evt->v7 << 8)) << 8))) =
                        _evt->v9 +
                        ((_evt->v10 +
                          ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                         << 8);
                    ++curr_seq_num;
                    break;
                case EVENT_ChangeGroup:
                    v38 =
                        _evt->v5 +
                        ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                         << 8);
                    v39 = _evt->v9 +
                          ((_evt->v10 +
                            ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                           << 8);
                    __debugbreak();
                    for (uint actor_id = 0; actor_id < uNumActors; actor_id++) {
                        if (pActors[actor_id].uGroup == v38)
                            pActors[actor_id].uGroup = v39;
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_ChangeGroupAlly:
                    v42 =
                        _evt->v5 +
                        ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                         << 8);
                    v43 = _evt->v9 +
                          ((_evt->v10 +
                            ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                           << 8);
                    __debugbreak();
                    for (uint actor_id = 0; actor_id < uNumActors; actor_id++) {
                        if (pActors[actor_id].uGroup == v42)
                            pActors[actor_id].uAlly = v43;
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_MoveNPC: {
                    pNPCStats->pNewNPCData[EVT_DWORD(_evt->v5)].Location2D =
                        EVT_DWORD(_evt->v9);
                    if (window_SpeakInHouse) {
                        if (window_SpeakInHouse->par1C == 165) {
                            HouseDialogPressCloseBtn();
                            pMediaPlayer->Unload();
                            window_SpeakInHouse->Release();
                            pParty->uFlags &= ~PARTY_FLAGS_1_0002;
                            activeLevelDecoration = (LevelDecoration *)1;
                            if (EnterHouse(HOUSE_BODY_GUILD_ERATHIA)) {
                                pAudioPlayer->PlaySound(SOUND_Invalid, 0, 0, -1, 0, 0);
                                window_SpeakInHouse = new GUIWindow_House(
                                    0, 0, window->GetWidth(),
                                    window->GetHeight(), (GUIButton *)165, 0);
                                window_SpeakInHouse->DeleteButtons();
                            }
                            //          else {
                            //            if ( window_SpeakInHouse->par1C == 553
                            //            ) {
                            //              pMediaPlayer->bLoopPlaying = 0;
                            //            }
                            //          }
                        }
                    }
                    ++curr_seq_num;
                    break;
                }
                case EVENT_Jmp:
                    curr_seq_num = _evt->v5 - 1;
                    ++curr_seq_num;
                    v4 = -1;

                    break;
                case EVENT_ShowFace:
                    if (_evt->v5 <= 3u) {  // someone
                        pParty->pPlayers[_evt->v5].PlayEmotion(
                            (CHARACTER_EXPRESSION_ID)_evt->v6, 0);
                    } else if (_evt->v5 == 4) {  // active
                        pParty->pPlayers[uActiveCharacter].PlayEmotion(
                            (CHARACTER_EXPRESSION_ID)_evt->v6, 0);
                    } else if (_evt->v5 == 5) {  // all players
                        for (int i = 0; i < 4; ++i)
                            pParty->pPlayers[i].PlayEmotion(
                                (CHARACTER_EXPRESSION_ID)_evt->v6, 0);
                    } else {  // random player
                        pParty->pPlayers[rand() % 4].PlayEmotion(
                            (CHARACTER_EXPRESSION_ID)_evt->v6, 0);
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_CharacterAnimation:
                    if (_evt->v5 <= 3) {  // someone
                        pParty->pPlayers[_evt->v5].PlaySound(
                            (PlayerSpeech)_evt->v6, 0);
                    } else if (_evt->v5 == 4) {  // active
                        pParty->pPlayers[uActiveCharacter].PlaySound(
                            (PlayerSpeech)_evt->v6, 0);
                    } else if (_evt->v5 == 5) {  // all
                        for (int i = 0; i < 4; ++i)
                            pParty->pPlayers[i].PlaySound(
                            (PlayerSpeech)_evt->v6, 0);
                    } else {  // random
                        pParty->pPlayers[rand() % 4].PlaySound(
                            (PlayerSpeech)_evt->v6, 0);
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_ForPartyMember:
                    player_choose = _evt->v5;
                    ++curr_seq_num;
                    break;
                case EVENT_SummonItem:
                    SpriteObject::Drop_Item_At(
                        (SPRITE_OBJECT_TYPE)(
                            _evt->v5 +
                            ((_evt->v6 +
                              ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                             << 8)),
                        _evt->v9 +
                            ((_evt->v10 +
                              ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                             << 8),
                        _evt->v13 +
                            ((_evt->v14 +
                              ((_evt->v15 + ((uint)_evt->v16 << 8)) << 8))
                             << 8),
                        _evt->v17 +
                            ((_evt->v18 +
                              ((_evt->v19 + ((uint)_evt->v20 << 8)) << 8))
                             << 8),
                        _evt->v21 +
                            ((_evt->v22 +
                              ((_evt->v23 + ((uint)_evt->v24 << 8)) << 8))
                             << 8),
                        _evt->v25, _evt->v26, 0, 0);
                    ++curr_seq_num;
                    break;
                case EVENT_Compare:
                    pValue = EVT_DWORD(_evt->v7);
                    if (player_choose <= 3) {
                        if (pPlayers[player_choose]->CompareVariable(
                                (enum VariableType)EVT_WORD(_evt->v5),
                                pValue)) {
                            // v124 = -1;
                            curr_seq_num = _evt->v11 - 1;
                        }
                    } else if (player_choose == 4) {  // active
                        if (uActiveCharacter) {
                            if (pPlayers[uActiveCharacter]->CompareVariable(
                                    (enum VariableType)EVT_WORD(_evt->v5),
                                    pValue)) {
                                // v124 = -1;
                                curr_seq_num = _evt->v11 - 1;
                            }
                        }
                    } else if (player_choose == 5) {  // all
                        v130 = 0;
                        for (int i = 1; i < 5; ++i) {
                            if (pPlayers[i]->CompareVariable(
                                    (enum VariableType)EVT_WORD(_evt->v5),
                                    pValue)) {
                                // v124 = -1;
                                curr_seq_num = _evt->v11 - 1;
                                break;
                            }
                            ++v130;
                        }
                    } else if (player_choose == 6) {  // random
                        if (pPlayers[rand() % 4 + 1]->CompareVariable(
                                (enum VariableType)EVT_WORD(_evt->v5),
                                pValue)) {
                            // v124 = -1;
                            curr_seq_num = _evt->v11 - 1;
                        }
                    }
                    ++curr_seq_num;
                    v4 = -1;
                    break;
                case EVENT_IsActorAlive:
                    if (IsActorAlive(EVT_BYTE(_evt->v5), EVT_DWORD(_evt->v6),
                                     EVT_BYTE(_evt->v10))) {
                        // v124 = -1;
                        curr_seq_num = _evt->v11 - 1;
                    }
                    ++curr_seq_num;
                    v4 = -1;
                    break;
                case EVENT_Substract:
                    pValue = EVT_DWORD(_evt->v7);
                    if (player_choose <= 3) {
                        pParty->pPlayers[player_choose].SubtractVariable(
                            (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 4) {  // active
                        if (uActiveCharacter)
                            pPlayers[uActiveCharacter]->SubtractVariable(
                                (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 5) {  // all
                        if (EVT_WORD(_evt->v5) == VAR_PlayerItemInHands) {
                            for (int i = 1; i < 5; ++i) {
                                if (pPlayers[i]->HasItem(pValue, 1)) {
                                    pPlayers[i]->SubtractVariable(
                                        (enum VariableType)EVT_WORD(_evt->v5), pValue);
                                    break;  // only take one item
                                }
                            }
                        } else {
                            for (int i = 1; i < 5; ++i)
                                pPlayers[i]->SubtractVariable(
                                (enum VariableType)EVT_WORD(_evt->v5), pValue);
                        }
                    } else if (player_choose == 6) {  // random
                        pParty->pPlayers[rand() % 4].SubtractVariable(
                            (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_Set:
                    pValue = EVT_DWORD(_evt->v7);
                    if (player_choose <= 3) {
                        pParty->pPlayers[player_choose].SetVariable(
                            (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 4) {  // active
                        if (uActiveCharacter)
                            pPlayers[uActiveCharacter]->SetVariable(
                                (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 5) {  // all
                        // recheck v130
                        for (int i = 1; i < 5; ++i)
                            pPlayers[i]->SetVariable(
                                (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 6) {  // random
                        pParty->pPlayers[rand() % 4].SetVariable(
                            (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_Add:
                    pValue = EVT_DWORD(_evt->v7);
                    if (player_choose <= 3) {
                        pPlayer = &pParty->pPlayers[player_choose];
                        pPlayer->AddVariable(
                            (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 4) {  // active
                        if (uActiveCharacter)
                            pPlayers[uActiveCharacter]->AddVariable(
                                (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    } else if (player_choose == 5) {  // all
                        for (int i = 1; i < 5; ++i) {
                            pPlayers[i]->AddVariable(
                                (enum VariableType)EVT_WORD(_evt->v5), pValue);
                        }
                    } else if (player_choose == 6) {  // random
                        pParty->pPlayers[rand() % 4].AddVariable(
                            (enum VariableType)EVT_WORD(_evt->v5), pValue);
                    }
                    v83 = EVT_WORD(_evt->v5);
                    if (v83 == 21 ||  // gold well on emerald isle
                        v83 == 22 || v83 == 23 || v83 == 24) {
                        // __debugbreak(); // bonfire
                        viewparams->bRedrawGameUI = true;
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_InputString:
                    if (!entry_line) {
                        game_ui_status_bar_event_string =
                            &pLevelStr[pLevelStrOffsets[EVT_DWORD(_evt->v5)]];
                        sub_4451A8_press_any_key(uEventID, curr_seq_num, 26);
                        if (v133 == 1) OnMapLeave();
                        return;
                    }
                    v84 = _evt->v13 +
                          ((_evt->v14 +
                            ((_evt->v15 + ((uint)_evt->v16 << 8)) << 8))
                           << 8);
                    if (!_stricmp(
                            game_ui_status_bar_event_string.data(),
                            &pLevelStr
                                [pLevelStrOffsets[_evt->v9 +
                                                  ((_evt->v10 +
                                                    ((_evt->v11 +
                                                      ((uint)_evt->v12 << 8))
                                                     << 8))
                                                   << 8)]]) ||
                        !_stricmp(game_ui_status_bar_event_string.data(),
                                  &pLevelStr[pLevelStrOffsets[v84]])) {
                        v11 = _evt->v17;
                        curr_seq_num = v11 - 1;
                    }
                    ++curr_seq_num;
                    v4 = -1;
                    break;
                case EVENT_RandomGoTo:
                    // v124 = -1;
                    v11 = (unsigned __int8)*(
                        &_evt->v5 +
                        rand() % ((_evt->v5 != 0) + (_evt->v6 != 0) +
                                  (_evt->v7 != 0) + (_evt->v8 != 0) +
                                  (_evt->v9 != 0) + (_evt->v10 != 0)));
                    curr_seq_num = v11 - 1;
                    ++curr_seq_num;
                    v4 = -1;
                    break;
                case EVENT_ReceiveDamage:
                    if ((unsigned __int8)_evt->v5 <= 3) {
                        pParty->pPlayers[(unsigned __int8)_evt->v5]
                            .ReceiveDamage(EVT_DWORD(_evt->v7),
                                           (DAMAGE_TYPE)_evt->v6);
                        ++curr_seq_num;
                        break;
                    }
                    if (_evt->v5 == 4) {
                        if (!uActiveCharacter) {
                            ++curr_seq_num;
                            break;
                        }
                        pPlayers[uActiveCharacter]->ReceiveDamage(
                            EVT_DWORD(_evt->v7), (DAMAGE_TYPE)_evt->v6);
                        ++curr_seq_num;
                        break;
                    }
                    if (_evt->v5 != 5) {
                        pParty->pPlayers[rand() % 4].ReceiveDamage(
                            EVT_DWORD(_evt->v7), (DAMAGE_TYPE)_evt->v6);
                        ++curr_seq_num;
                        break;
                    }
                    for (uint pl_id = 0; pl_id < 4; pl_id++)
                        pParty->pPlayers[pl_id].ReceiveDamage(
                            EVT_DWORD(_evt->v7), (DAMAGE_TYPE)_evt->v6);
                    ++curr_seq_num;
                    break;
                case EVENT_ToggleIndoorLight:
                    pIndoor->ToggleLight(EVT_DWORD(_evt->v5), _evt->v9);
                    ++curr_seq_num;
                    break;
                case EVENT_SetFacesBit:
                    sub_44892E_set_faces_bit(EVT_DWORD(_evt->v5),
                                             EVT_DWORD(_evt->v9), _evt->v13);
                    ++curr_seq_num;
                    break;
                case EVENT_ToggleChestFlag:
                    Chest::ToggleFlag(EVT_DWORD(_evt->v5), EVT_DWORD(_evt->v9),
                                      _evt->v13);
                    ++curr_seq_num;
                    break;
                case EVENT_ToggleActorFlag:
                    Actor::ToggleFlag(EVT_DWORD(_evt->v5), EVT_DWORD(_evt->v9),
                                      _evt->v13);
                    ++curr_seq_num;
                    break;
                case EVENT_ToggleActorGroupFlag:
                    ToggleActorGroupFlag(EVT_DWORD(_evt->v5),
                                         EVT_DWORD(_evt->v9), _evt->v13);
                    ++curr_seq_num;
                    break;
                case EVENT_SetSnow:
                    if (!_evt->v5) pWeather->bRenderSnow = _evt->v6 != 0;
                    ++curr_seq_num;
                    break;
                case EVENT_StatusText:
                    v90 = EVT_DWORD(_evt->v5);
                    if (activeLevelDecoration) {
                        if (activeLevelDecoration == (LevelDecoration *)1)
                            current_npc_text =
                                pNPCTopics[v90 - 1]
                                    .pText;  // (&dword_721664)[8 * v90];
                        if (canShowMessages == 1) {
                            v91 = pNPCTopics[v90 - 1]
                                      .pText;  // (&dword_721664)[8 * v90];
                            // LABEL_248:
                            GameUI_StatusBar_OnEvent(v91, 2);
                        }
                    } else {
                        if (canShowMessages == 1) {
                            v91 = &pLevelStr[pLevelStrOffsets[v90]];
                            GameUI_StatusBar_OnEvent(v91, 2);
                        }
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_ShowMessage:
                    if (activeLevelDecoration) {
                        current_npc_text =
                            pNPCTopics[EVT_DWORD(_evt->v5) - 1]
                                .pText;  // (&dword_721664)[8 * v92];
                        branchless_dialogue_str.clear();
                    } else {
                        branchless_dialogue_str = &pLevelStr[pLevelStrOffsets[EVT_DWORD(_evt->v5)]];
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_CastSpell:
                    EventCastSpell(_evt->v5, _evt->v6, _evt->v7,
                                   EVT_DWORD(_evt->v8), EVT_DWORD(_evt->v12),
                                   EVT_DWORD(_evt->v16), EVT_DWORD(_evt->v20),
                                   EVT_DWORD(_evt->v24), EVT_DWORD(_evt->v28));
                    ++curr_seq_num;
                    break;
                case EVENT_SetTexture:
                    sub_44861E_set_texture(EVT_DWORD(_evt->v5),
                                           (char *)&_evt->v9);
                    ++curr_seq_num;
                    break;
                case EVENT_SetSprite:
                    SetDecorationSprite(EVT_DWORD(_evt->v5), _evt->v9,
                                        (char *)&_evt->v10);
                    ++curr_seq_num;
                    break;
                case EVENT_SummonMonsters:
                    sub_448CF4_spawn_monsters(
                        _evt->v5, _evt->v6, _evt->v7, EVT_DWORD(_evt->v8),
                        EVT_DWORD(_evt->v12), EVT_DWORD(_evt->v16),
                        EVT_DWORD(_evt->v20), EVT_DWORD(_evt->v24));
                    ++curr_seq_num;
                    break;
                case EVENT_MouseOver:
                case EVENT_LocationName:
                    --curr_seq_num;  // eh?
                    ++curr_seq_num;
                    break;
                case EVENT_ChangeDoorState:
                    Door_switch_animation(_evt->v5, _evt->v6);
                    ++curr_seq_num;
                    break;
                case EVENT_OpenChest:
                    if (!Chest::Open(_evt->v5)) {
                        if (v133 == 1) OnMapLeave();
                        return;
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_MoveToMap:
                    v94 =
                        _evt->v5 +
                        ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                         << 8);
                    v135 =
                        _evt->v5 +
                        ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                         << 8);
                    v132 = _evt->v9 +
                           ((_evt->v10 +
                             ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                            << 8);
                    v126 = _evt->v13 +
                           ((_evt->v14 +
                             ((_evt->v15 + ((uint)_evt->v16 << 8)) << 8))
                            << 8);
                    v129 = _evt->v17 +
                           ((_evt->v18 +
                             ((_evt->v19 + ((uint)_evt->v20 << 8)) << 8))
                            << 8);
                    v95 = _evt->v21 +
                          ((_evt->v22 +
                            ((_evt->v23 + ((uint)_evt->v24 << 8)) << 8))
                           << 8);
                    v96 = _evt->v25;
                    v97 = v96 + ((_evt->v26 +
                                  ((_evt->v27 + ((uint)_evt->v28 << 8)) << 8))
                                 << 8);
                    v134 = v96 + ((_evt->v26 +
                                   ((_evt->v27 + ((uint)_evt->v28 << 8)) << 8))
                                  << 8);
                    if (_evt->v29 || _evt->v30) {
                        pDialogueWindow = new GUIWindow_Transition(
                            _evt->v29, _evt->v30, v135, v132, v126, v129, v95,
                            v134, (char *)&_evt->v31);
                        dword_5C3418 = uEventID;
                        dword_5C341C = curr_seq_num + 1;
                        if (v133 == 1) OnMapLeave();
                        return;
                    }
                    Party_Teleport_Y_Pos =
                        _evt->v9 +
                        ((_evt->v10 +
                          ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                         << 8);
                    Party_Teleport_X_Pos = v94;
                    Party_Teleport_Z_Pos = v126;
                    if (v129 == -1) {
                        v98 = Party_Teleport_Cam_Yaw;
                    } else {
                        v98 = v129 & TrigLUT->uDoublePiMask;
                        Party_Teleport_Cam_Yaw =
                            v129 & TrigLUT->uDoublePiMask;
                    }
                    v99 = (char *)&_evt->v31;
                    Party_Teleport_Cam_Pitch = v95;
                    Party_Teleport_Z_Speed = v97;
                    v100 = v94 | v132 | v126 | v95 | v97 | v98;
                    Start_Party_Teleport_Flag = v100;
                    if (*v99 == 48) {
                        if (v100) {
                            pParty->vPosition.x = v135;
                            pParty->vPosition.y = v132;
                            pParty->vPosition.z = v126;
                            pParty->uFallStartY = v126;
                            if (Party_Teleport_Cam_Yaw != -1)
                                pParty->sRotationZ =
                                    Party_Teleport_Cam_Yaw;
                            Party_Teleport_Cam_Yaw = -1;
                            pParty->sRotationX = v95;
                            pParty->uFallSpeed = v134;
                            Start_Party_Teleport_Flag = 0;
                            Party_Teleport_Z_Speed = 0;
                            Party_Teleport_Cam_Pitch = 0;
                            Party_Teleport_Z_Pos = 0;
                            Party_Teleport_Y_Pos = 0;
                            Party_Teleport_X_Pos = 0;
                            v106 = 232;
                            pAudioPlayer->PlaySound((SoundID)v106, 0, 0, -1, 0, 0);
                        }
                    } else {
                        pGameLoadingUI_ProgressBar->Initialize((GUIProgressBar::Type)((activeLevelDecoration == NULL) + 1));
                        Transition_StopSound_Autosave(v99, MapStartPoint_Party);
                        v133 = 1;
                        if (current_screen_type == CURRENT_SCREEN::SCREEN_HOUSE) {
                            if (uGameState == GAME_STATE_CHANGE_LOCATION) {
                                pAudioPlayer->StopChannels(-1, -1);
                                dialog_menu_id = HOUSE_DIALOGUE_NULL;
                                while (HouseDialogPressCloseBtn());
                                pMediaPlayer->Unload();
                                window_SpeakInHouse->Release();
                                window_SpeakInHouse = 0;
                                pMessageQueue_50CBD0->Flush();
                                current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
                                viewparams->bRedrawGameUI = 1;
                                pDialogueNPCCount = 0;
                                if (pDialogueWindow) {
                                    pDialogueWindow->Release();
                                    pDialogueWindow = 0;
                                }
                                dialog_menu_id = HOUSE_DIALOGUE_NULL;
                                pIcons_LOD->SyncLoadedFilesCount();
                            }
                            OnMapLeave();
                            return;
                        }
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_PlaySound:
                    v110 = _evt->v13 +
                           ((_evt->v14 +
                             ((_evt->v15 + ((uint)_evt->v16 << 8)) << 8))
                            << 8);
                    v109 = _evt->v9 +
                           ((_evt->v10 +
                             ((_evt->v11 + ((uint)_evt->v12 << 8)) << 8))
                            << 8);
                    v106 =
                        _evt->v5 +
                        ((_evt->v6 + ((_evt->v7 + ((uint)_evt->v8 << 8)) << 8))
                         << 8);
                    pAudioPlayer->PlaySound((SoundID)v106, 0, 0, v109, v110, 0);
                    ++curr_seq_num;
                    break;
                case EVENT_GiveItem:
                    item.Reset();
                    v102 =
                        _evt->v7 +
                        ((_evt->v8 + ((_evt->v9 + ((uint)_evt->v10 << 8)) << 8))
                         << 8);
                    pItemsTable->GenerateItem(_evt->v5, _evt->v6, &item);
                    if (v102) item.uItemID = v102;
                    pParty->SetHoldingItem(&item);
                    ++curr_seq_num;
                    break;
                case EVENT_SpeakInHouse:
                    if (EnterHouse((enum HOUSE_ID)EVT_DWORD(_evt->v5))) {
                        pAudioPlayer->PlaySound(SOUND_Invalid, 0, 0, -1, 0, 0);
                        pAudioPlayer->PlaySound(SOUND_enter, 814, 0, -1, 0, 0);
                        v104 = 187;
                        if (uCurrentHouse_Animation != 167)
                            v104 = EVT_DWORD(_evt->v5);
                        window_SpeakInHouse =
                            new GUIWindow_House(0, 0, window->GetWidth(),
                                                window->GetHeight(), (GUIButton *)v104);
                        window_SpeakInHouse->CreateButton(61, 424, 31, 0, 2, 94,
                                                          UIMSG_SelectCharacter,
                                                          1, GameKey::Digit1, "");
                        window_SpeakInHouse->CreateButton(
                            177, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 2,
                            GameKey::Digit2, "");
                        window_SpeakInHouse->CreateButton(
                            292, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 3,
                            GameKey::Digit3, "");
                        window_SpeakInHouse->CreateButton(
                            407, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 4,
                            GameKey::Digit4, "");
                        window_SpeakInHouse->CreateButton(0, 0, 0, 0, 1, 0,
                                                          UIMSG_CycleCharacters,
                                                          0, GameKey::Tab, "");
                    }
                    ++curr_seq_num;
                    break;
                case EVENT_PressAnyKey:
                    sub_4451A8_press_any_key(uEventID, curr_seq_num + 1, 33);
                    if (v133 == 1) OnMapLeave();
                    return;
                case EVENT_Exit:
                    if (v133 == 1) OnMapLeave();
                    return;
                default:
                    ++curr_seq_num;
                    break;
            }
        }
    }
    if (v133 == 1) OnMapLeave();
    return;
}

//----- (00444732) --------------------------------------------------------
char *GetEventHintString(unsigned int uEventID) {
    signed int event_index;  // edx@1
    int event_pos;           // esi@4
    char *result;            // eax@6
    unsigned int str_index;  // eax@9
    int i;                   // esi@11
    _evt_raw *test_evt;
    _evt_raw *last_evt;

    event_index = 0;
    if (uLevelEVT_NumEvents <= 0) return NULL;

    // v2 = (char *)&pLevelEVT_Index[0].uEventOffsetInEVT;
    while (1) {
        if (pLevelEVT_Index[event_index].uEventID == uEventID) {
            test_evt = (_evt_raw *)&pLevelEVT[pLevelEVT_Index[event_index]
                                                  .uEventOffsetInEVT];
            last_evt = test_evt;
            event_pos = pLevelEVT_Index[event_index + 1].uEventOffsetInEVT;
            if (test_evt->_e_type == EVENT_MouseOver) break;
        }
        ++event_index;
        if (event_index >= uLevelEVT_NumEvents) return NULL;
    }
    test_evt = (_evt_raw *)&pLevelEVT[event_pos];
    if (test_evt->_e_type == EVENT_SpeakInHouse) {
        str_index = EVT_DWORD(test_evt->v5);
        result = (char *)p2DEvents[str_index - 1].pName;
    } else {
        for (i = event_index + 1; pLevelEVT_Index[i].uEventID == uEventID;
             ++i) {
            event_pos = pLevelEVT_Index[i].uEventOffsetInEVT;
            test_evt = (_evt_raw *)&pLevelEVT[event_pos];
            if (test_evt->_e_type == EVENT_SpeakInHouse) {
                str_index = EVT_DWORD(test_evt->v5);
                if (str_index < 525)  // 600
                    return (char *)p2DEvents[str_index - 1].pName;
            }
        }
        result = &pLevelStr[pLevelStrOffsets[EVT_BYTE(last_evt->v5)]];
    }

    return result;
}

//----- (004613C4) --------------------------------------------------------
void init_event_triggers() {
    uint id = pDecorationList->GetDecorIdByName("Event Trigger");

    num_event_triggers = 0;
    for (uint i = 0; i < uNumLevelDecorations; ++i)
        if (pLevelDecorations[i].uDecorationDescID == id)
            event_triggers[num_event_triggers++] = i;
}

//----- (0046CC4B) --------------------------------------------------------
void check_event_triggers() {
    for (size_t i = 0; i < num_event_triggers; i++) {
        if (pLevelDecorations[event_triggers[i]].uFlags &
                LEVEL_DECORATION_TRIGGERED_BY_TOUCH &&
            pLevelDecorations[event_triggers[i]].vPosition.GetDistanceTo(
                pParty->vPosition) <
                pLevelDecorations[event_triggers[i]].uTriggerRange) {
            EventProcessor(pLevelDecorations[event_triggers[i]].uEventID,
                           PID(OBJECT_Decoration, i), 1);
        } else if (pLevelDecorations[event_triggers[i]].uFlags &
                   LEVEL_DECORATION_TRIGGERED_BY_MONSTER) {
            for (size_t j = 0; j < uNumActors; j++) {
                if (pLevelDecorations[event_triggers[i]]
                        .vPosition.GetDistanceTo(pActors[j].vPosition) <
                    pLevelDecorations[event_triggers[i]].uTriggerRange)
                    EventProcessor(
                        pLevelDecorations[event_triggers[i]].uEventID, 0, 1);
            }
        } else if (pLevelDecorations[event_triggers[i]].uFlags &
                   LEVEL_DECORATION_TRIGGERED_BY_OBJECT) {
            for (size_t j = 0; j < uNumSpriteObjects; j++) {
                if (pLevelDecorations[event_triggers[i]]
                        .vPosition.GetDistanceTo(pSpriteObjects[j].vPosition) <
                    pLevelDecorations[event_triggers[i]].uTriggerRange)
                    EventProcessor(
                        pLevelDecorations[event_triggers[i]].uEventID, 0, 1);
            }
        }
    }
}

//----- (004465DF) --------------------------------------------------------
bool sub_4465DF_check_season(int a1) {
    auto monthPlusOne = pParty->uCurrentMonth + 1;
    auto daysPlusOne = pParty->uCurrentDayOfMonth + 1;

    switch (a1) {
        case 3:  // winter 12.21 -> 3.20
            return (monthPlusOne == 12 && daysPlusOne >= 21 ||
                    monthPlusOne == 1 || monthPlusOne == 2 ||
                    monthPlusOne == 3 && daysPlusOne <= 20);

        case 2:  // autumn/fall 9.21 -> 12.20
            return (monthPlusOne == 9 && daysPlusOne >= 21 ||
                    monthPlusOne == 10 || monthPlusOne == 11 ||
                    monthPlusOne == 12 && daysPlusOne <= 20);

        case 1:  // summer 6.21 -> 9.20
            return (monthPlusOne == 6 && daysPlusOne >= 21 ||
                    monthPlusOne == 7 || monthPlusOne == 8 ||
                    monthPlusOne == 9 && daysPlusOne <= 20);

        case 0:  // spring 3.21 -> 6.20
            return (monthPlusOne == 3 && daysPlusOne >= 21 ||
                    monthPlusOne == 4 || monthPlusOne == 5 ||
                    monthPlusOne == 6 && daysPlusOne <= 20);

        default:
            Error("Unknown season");
    }

    return false;
}

//----- (00448CF4) --------------------------------------------------------
void sub_448CF4_spawn_monsters(__int16 typeindex, __int16 level, int count,
                               int x, int y, int z, int group,
                               unsigned int uUniqueName) {
    unsigned int map_id;        // eax@1
    size_t old_num_actors;      // ebx@2
    AIDirection v15;            // [sp+28h] [bp-34h]@2
    SpawnPointMM7 pSpawnPoint;  // [sp+44h] [bp-18h]@1

    pSpawnPoint.vPosition.x = x;
    pSpawnPoint.vPosition.y = y;
    pSpawnPoint.vPosition.z = z;
    pSpawnPoint.uGroup = group;
    pSpawnPoint.uRadius = 32;
    pSpawnPoint.uKind = 3;
    pSpawnPoint.uIndex = typeindex + 2 * level + level;
    map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
        old_num_actors = uNumActors;
        SpawnEncounter(&pMapStats->pInfos[map_id], &pSpawnPoint, 0, count, 0);
        Actor::GetDirectionInfo(PID(OBJECT_Actor, old_num_actors), 4, &v15, 1);
        for (uint i = (unsigned int)old_num_actors;
             i < (unsigned int)uNumActors; ++i) {
            pActors[i].PrepareSprites(0);
            pActors[i].uYawAngle = v15.uYawAngle;
            pActors[i].dword_000334_unique_name = uUniqueName;
        }
    }
}
