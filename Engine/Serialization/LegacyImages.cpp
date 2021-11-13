#include <algorithm>

#include "Engine/Serialization/LegacyImages.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Time.h"

SpriteFrame_MM6::SpriteFrame_MM6() {
    Assert(sizeof(*this) == 56);
    memset(this, 0, sizeof(*this));
}

SpriteFrame_MM7::SpriteFrame_MM7() {
    Assert(sizeof(*this) == 60);
    memset(this, 0, sizeof(*this));
}

NPCData_Image_MM7::NPCData_Image_MM7() {
    Assert(sizeof(*this) == 0x4C);
    memset(this, 0, sizeof(*this));
}

ItemGen_Image_MM7::ItemGen_Image_MM7() {
    Assert(sizeof(*this) == 0x24);
    memset(this, 0, sizeof(*this));
}

SpellBuff_Image_MM7::SpellBuff_Image_MM7() {
    Assert(sizeof(*this) == 0x10);
    memset(this, 0, sizeof(*this));
}

PlayerSpellbookChapter_Image_MM7::PlayerSpellbookChapter_Image_MM7() {
    Assert(sizeof(*this) == 0xB);
    memset(this, 0, sizeof(*this));
}

PlayerSpells_Image_MM7::PlayerSpells_Image_MM7() {
    Assert(sizeof(*this) == 0x64);
    memset(this, 0, sizeof(*this));
}

PlayerEquipment_Image_MM7::PlayerEquipment_Image_MM7() {
    Assert(sizeof(*this) == 0x40);
    memset(this, 0, sizeof(*this));
}

LloydBeacon_Image_MM7::LloydBeacon_Image_MM7() {
    Assert(sizeof(*this) == 0x1C);
    memset(this, 0, sizeof(*this));
}

Player_Image_MM7::Player_Image_MM7() {
    Assert(sizeof(*this) == 0x1B3C);
    memset(this, 0, sizeof(*this));
}

PartyTimeStruct_Image_MM7::PartyTimeStruct_Image_MM7() {
    Assert(sizeof(*this) == 0x678);
    memset(this, 0, sizeof(*this));
}

Party_Image_MM7::Party_Image_MM7() {
    Assert(sizeof(*this) == 0x16238);
    memset(this, 0, sizeof(*this));
}

Timer_Image_MM7::Timer_Image_MM7() {
    Assert(sizeof(*this) == 0x28);
    memset(this, 0, sizeof(*this));
}

OtherOverlay_Image_MM7::OtherOverlay_Image_MM7() {
    Assert(sizeof(*this) == 0x14);
    memset(this, 0, sizeof(*this));
}

OtherOverlayList_Image_MM7::OtherOverlayList_Image_MM7() {
    Assert(sizeof(*this) == 0x3F0);
    memset(this, 0, sizeof(*this));
}

IconFrame_MM7::IconFrame_MM7() {
    Assert(sizeof(*this) == 0x20);
    memset(this, 0, sizeof(*this));
}

UIAnimation_MM7::UIAnimation_MM7() {
    Assert(sizeof(*this) == 0xD);
    memset(this, 0, sizeof(*this));
}

MonsterInfo_MM7::MonsterInfo_MM7() {
    Assert(sizeof(*this) == 0x58);
    memset(this, 0, sizeof(*this));
}

Actor_MM7::Actor_MM7() {
    Assert(sizeof(*this) == 0x344);
    memset(this, 0, sizeof(*this));
}

BLVSector_MM7::BLVSector_MM7() {
    Assert(sizeof(*this) == 0x74);
    memset(this, 0, sizeof(*this));
}

FontData_MM7::FontData_MM7() {
    Assert(sizeof(*this) == 0x1020);
    memset(this, 0, sizeof(*this));
}

void Timer_Image_MM7::Serialize(Timer *timer) {
    memset(this, 0, sizeof(*this));

    this->bReady = timer->bReady;
    this->bPaused = timer->bPaused;
    this->bTackGameTime = timer->bTackGameTime;
    this->uStartTime = timer->uStartTime;
    this->uStopTime = timer->uStopTime;
    this->uGameTimeStart = timer->uGameTimeStart;
    this->field_18 = timer->field_18;
    this->uTimeElapsed = timer->uTimeElapsed;
    this->dt_in_some_format = timer->dt_in_some_format;
    this->uTotalGameTimeElapsed = timer->uTotalGameTimeElapsed;
}

void Timer_Image_MM7::Deserialize(Timer *timer) {
    timer->bReady = this->bReady;
    timer->bPaused = this->bPaused;
    timer->bTackGameTime = this->bTackGameTime;
    timer->uStartTime = this->uStartTime;
    timer->uStopTime = this->uStopTime;
    timer->uGameTimeStart = this->uGameTimeStart;
    timer->field_18 = this->field_18;
    timer->uTimeElapsed = this->uTimeElapsed;
    timer->dt_in_some_format = this->dt_in_some_format;
    timer->uTotalGameTimeElapsed = this->uTotalGameTimeElapsed;
}

void NPCData_Image_MM7::Serialize(NPCData *npc) {
    memset(this, 0, sizeof(*this));

    if (npc->pName) {
        this->pName = 1;
    } else {
        this->pName = 0;
    }
    // this->pName = npc->pName;
    this->uPortraitID = npc->uPortraitID;
    this->uFlags = npc->uFlags;
    this->fame = npc->fame;
    this->rep = npc->rep;
    this->Location2D = npc->Location2D;
    this->uProfession = npc->uProfession;
    this->greet = npc->greet;
    this->joins = npc->joins;
    this->field_24 = npc->field_24;
    this->evt_A = npc->evt_A;
    this->evt_B = npc->evt_B;
    this->evt_C = npc->evt_C;
    this->evt_D = npc->evt_D;
    this->evt_E = npc->evt_E;
    this->evt_F = npc->evt_F;
    this->uSex = npc->uSex;
    this->bHasUsedTheAbility = npc->bHasUsedTheAbility;
    this->news_topic = npc->news_topic;
}

void NPCData_Image_MM7::Deserialize(NPCData *npc) {
    if (this->pName) {
        npc->pName = "Dummy";
    } else {
        npc->pName = nullptr;
    }
    // npc->pName = this->pName;
    npc->uPortraitID = this->uPortraitID;
    npc->uFlags = this->uFlags;
    npc->fame = this->fame;
    npc->rep = this->rep;
    npc->Location2D = this->Location2D;
    npc->uProfession = this->uProfession;
    npc->greet = this->greet;
    npc->joins = this->joins;
    npc->field_24 = this->field_24;
    npc->evt_A = this->evt_A;
    npc->evt_B = this->evt_B;
    npc->evt_C = this->evt_C;
    npc->evt_D = this->evt_D;
    npc->evt_E = this->evt_E;
    npc->evt_F = this->evt_F;
    npc->uSex = this->uSex;
    npc->bHasUsedTheAbility = this->bHasUsedTheAbility;
    npc->news_topic = this->news_topic;
}

void OtherOverlayList_Image_MM7::Serialize(OtherOverlayList *list) {
    memset(this, 0, sizeof(*this));

    this->bRedraw = list->bRedraw;
    this->field_3E8 = list->field_3E8;

    for (unsigned int i = 0; i < 50; ++i) {
        memset(&this->pOverlays[i], 0, sizeof(this->pOverlays[i]));

        this->pOverlays[i].field_0 = list->pOverlays[i].field_0;
        this->pOverlays[i].field_2 = list->pOverlays[i].field_2;
        this->pOverlays[i].sprite_frame_time =
            list->pOverlays[i].sprite_frame_time;
        this->pOverlays[i].field_6 = list->pOverlays[i].field_6;
        this->pOverlays[i].screen_space_x = list->pOverlays[i].screen_space_x;
        this->pOverlays[i].screen_space_y = list->pOverlays[i].screen_space_y;
        this->pOverlays[i].field_C = list->pOverlays[i].field_C;
        this->pOverlays[i].field_E = list->pOverlays[i].field_E;
        this->pOverlays[i].field_10 = list->pOverlays[i].field_10;
    }
}

void OtherOverlayList_Image_MM7::Deserialize(OtherOverlayList *list) {
    list->bRedraw = this->bRedraw;
    list->field_3E8 = this->field_3E8;

    for (unsigned int i = 0; i < 50; ++i) {
        memset(&list->pOverlays[i], 0, sizeof(list->pOverlays[i]));

        list->pOverlays[i].field_0 = this->pOverlays[i].field_0;
        list->pOverlays[i].field_2 = this->pOverlays[i].field_2;
        list->pOverlays[i].sprite_frame_time =
            this->pOverlays[i].sprite_frame_time;
        list->pOverlays[i].field_6 = this->pOverlays[i].field_6;
        list->pOverlays[i].screen_space_x = this->pOverlays[i].screen_space_x;
        list->pOverlays[i].screen_space_y = this->pOverlays[i].screen_space_y;
        list->pOverlays[i].field_C = this->pOverlays[i].field_C;
        list->pOverlays[i].field_E = this->pOverlays[i].field_E;
        list->pOverlays[i].field_10 = this->pOverlays[i].field_10;
    }
}

void SpellBuff_Image_MM7::Serialize(SpellBuff *buff) {
    memset(this, 0, sizeof(*this));

    this->uExpireTime = buff->expire_time.value;
    this->uPower = buff->uPower;
    this->uSkill = buff->uSkill;
    this->uOverlayID = buff->uOverlayID;
    this->uCaster = buff->uCaster;
    this->uFlags = buff->uFlags;
}

void SpellBuff_Image_MM7::Deserialize(SpellBuff *buff) {
    buff->expire_time.value = this->uExpireTime;
    buff->uPower = this->uPower;
    buff->uSkill = this->uSkill;
    buff->uOverlayID = this->uOverlayID;
    buff->uCaster = this->uCaster;
    buff->uFlags = this->uFlags;
}

void ItemGen_Image_MM7::Serialize(ItemGen *item) {
    memset(this, 0, sizeof(*this));

    this->uItemID = item->uItemID;
    this->uEnchantmentType = item->uEnchantmentType;
    this->m_enchantmentStrength = item->m_enchantmentStrength;
    this->special_enchantment = item->special_enchantment;
    this->uNumCharges = item->uNumCharges;
    this->uAttributes = item->uAttributes;
    this->uBodyAnchor = item->uBodyAnchor;
    this->uMaxCharges = item->uMaxCharges;
    this->uHolderPlayer = item->uHolderPlayer;
    this->field_1B = item->field_1B;
    this->uExpireTime = item->uExpireTime.value;
}

void ItemGen_Image_MM7::Deserialize(ItemGen *item) {
    item->uItemID = this->uItemID;
    item->uEnchantmentType = this->uEnchantmentType;
    item->m_enchantmentStrength = this->m_enchantmentStrength;
    item->special_enchantment = (ITEM_ENCHANTMENT)this->special_enchantment;
    item->uNumCharges = this->uNumCharges;
    item->uAttributes = this->uAttributes;
    item->uBodyAnchor = this->uBodyAnchor;
    item->uMaxCharges = this->uMaxCharges;
    item->uHolderPlayer = this->uHolderPlayer;
    item->field_1B = this->field_1B;
    item->uExpireTime.value = this->uExpireTime;
}

void Party_Image_MM7::Serialize(Party *party) {
    memset(this, 0, sizeof(*this));

    this->field_0 = party->field_0;
    this->uPartyHeight = party->uPartyHeight;
    this->uDefaultPartyHeight = party->uDefaultPartyHeight;
    this->sEyelevel = party->sEyelevel;
    this->uDefaultEyelevel = party->uDefaultEyelevel;
    this->field_14_radius = party->field_14_radius;
    this->y_rotation_granularity = party->y_rotation_granularity;
    this->uWalkSpeed = party->uWalkSpeed;
    this->y_rotation_speed = party->y_rotation_speed;
    this->field_24 = party->field_24;
    this->field_28 = party->field_28;
    this->uTimePlayed = party->playing_time.value;
    this->uLastRegenerationTime = party->last_regenerated.value;

    for (unsigned int i = 0; i < 10; ++i)
        this->PartyTimes.bountyHunting_next_generation_time[i] =
            party->PartyTimes.bountyHunting_next_generation_time[i];
    for (unsigned int i = 0; i < 85; ++i)
        this->PartyTimes.Shops_next_generation_time[i] =
            party->PartyTimes.Shops_next_generation_time[i];
    for (unsigned int i = 0; i < 53; ++i)
        this->PartyTimes._shop_ban_times[i] =
            party->PartyTimes._shop_ban_times[i];
    for (unsigned int i = 0; i < 10; ++i)
        this->PartyTimes.CounterEventValues[i] =
            party->PartyTimes.CounterEventValues[i];
    for (unsigned int i = 0; i < 29; ++i)
        this->PartyTimes.HistoryEventTimes[i] =
            party->PartyTimes.HistoryEventTimes[i];
    for (unsigned int i = 0; i < 20; ++i)
        this->PartyTimes._s_times[i] = party->PartyTimes._s_times[i];

    this->vPosition.x = party->vPosition.x;
    this->vPosition.y = party->vPosition.y;
    this->vPosition.z = party->vPosition.z;
    this->sRotationZ = party->sRotationZ;
    this->sRotationX = party->sRotationX;
    this->vPrevPosition.x = party->vPrevPosition.x;
    this->vPrevPosition.y = party->vPrevPosition.y;
    this->vPrevPosition.z = party->vPrevPosition.z;
    this->sPrevRotationY = party->sPrevRotationY;
    this->sPrevRotationX = party->sPrevRotationX;
    this->sPrevEyelevel = party->sPrevEyelevel;
    this->field_6E0 = party->field_6E0;
    this->field_6E4 = party->field_6E4;
    this->uFallSpeed = party->uFallSpeed;
    this->field_6EC = party->field_6EC;
    this->field_6F0 = party->field_6F0;
    this->floor_face_pid = party->floor_face_pid;
    this->walk_sound_timer = party->walk_sound_timer;
    this->_6FC_water_lava_timer = party->_6FC_water_lava_timer;
    this->uFallStartY = party->uFallStartY;
    this->bFlying = party->bFlying;
    this->field_708 = party->field_708;
    this->hirelingScrollPosition = party->hirelingScrollPosition;
    this->field_70A = party->cNonHireFollowers;
    this->field_70B = party->field_70B;
    this->uCurrentYear = party->uCurrentYear;
    this->uCurrentMonth = party->uCurrentMonth;
    this->uCurrentMonthWeek = party->uCurrentMonthWeek;
    this->uCurrentDayOfMonth = party->uCurrentDayOfMonth;
    this->uCurrentHour = party->uCurrentHour;
    this->uCurrentMinute = party->uCurrentMinute;
    this->uCurrentTimeSecond = party->uCurrentTimeSecond;
    this->uNumFoodRations = party->GetFood();
    this->field_72C = party->field_72C;
    this->field_730 = party->field_730;
    this->uNumGold = party->GetGold();
    this->uNumGoldInBank = party->uNumGoldInBank;
    this->uNumDeaths = party->uNumDeaths;
    this->field_740 = party->field_740;
    this->uNumPrisonTerms = party->uNumPrisonTerms;
    this->uNumBountiesCollected = party->uNumBountiesCollected;
    this->field_74C = party->field_74C;

    for (unsigned int i = 0; i < 5; ++i)
        this->monster_id_for_hunting[i] = party->monster_id_for_hunting[i];
    for (unsigned int i = 0; i < 5; ++i)
        this->monster_for_hunting_killed[i] =
            party->monster_for_hunting_killed[i];

    this->days_played_without_rest = party->days_played_without_rest;

    for (unsigned int i = 0; i < 64; ++i)
        this->_quest_bits[i] = party->_quest_bits[i];
    for (unsigned int i = 0; i < 16; ++i)
        this->pArcomageWins[i] = party->pArcomageWins[i];

    this->field_7B5_in_arena_quest = party->field_7B5_in_arena_quest;
    this->uNumArenaPageWins = party->uNumArenaPageWins;
    this->uNumArenaSquireWins = party->uNumArenaSquireWins;
    this->uNumArenaKnightWins = party->uNumArenaKnightWins;
    this->uNumArenaLordWins = party->uNumArenaLordWins;

    for (unsigned int i = 0; i < 29; ++i)
        this->pIsArtifactFound[i] = party->pIsArtifactFound[i];
    for (unsigned int i = 0; i < 39; ++i)
        this->field_7d7[i] = party->field_7d7[i];
    for (unsigned int i = 0; i < 26; ++i)
        this->_autonote_bits[i] = party->_autonote_bits[i];
    for (unsigned int i = 0; i < 60; ++i)
        this->field_818[i] = party->field_818[i];
    for (unsigned int i = 0; i < 32; ++i)
        this->field_854[i] = party->field_854[i];

    this->uNumArcomageWins = party->uNumArcomageWins;
    this->uNumArcomageLoses = party->uNumArcomageLoses;
    this->bTurnBasedModeOn = party->bTurnBasedModeOn;
    this->field_880 = party->field_880;
    this->uFlags2 = party->uFlags2;

    uint align = 0;
    if (party->alignment == PartyAlignment::PartyAlignment_Evil) align = 2;
    if (party->alignment == PartyAlignment::PartyAlignment_Neutral) align = 1;
    this->alignment = align;

    for (unsigned int i = 0; i < 20; ++i)
        this->pPartyBuffs[i].Serialize(&party->pPartyBuffs[i]);
    for (unsigned int i = 0; i < 4; ++i)
        this->pPlayers[i].Serialize(&party->pPlayers[i]);
    for (unsigned int i = 0; i < 2; ++i)
        this->pHirelings[i].Serialize(&party->pHirelings[i]);

    this->pPickedItem.Serialize(&party->pPickedItem);

    this->uFlags = party->uFlags;

    for (unsigned int i = 0; i < 53; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            this->StandartItemsInShops[i][j].Serialize(
                &party->StandartItemsInShops[i][j]);

    for (unsigned int i = 0; i < 53; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            this->SpecialItemsInShops[i][j].Serialize(
                &party->SpecialItemsInShops[i][j]);

    for (unsigned int i = 0; i < 32; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            this->SpellBooksInGuilds[i][j].Serialize(
                &party->SpellBooksInGuilds[i][j]);

    for (unsigned int i = 0; i < 24; ++i)
        this->field_1605C[i] = party->field_1605C[i];

    strcpy(this->pHireling1Name, party->pHireling1Name);
    strcpy(this->pHireling2Name, party->pHireling2Name);

    this->armageddon_timer = party->armageddon_timer;
    this->armageddonDamage = party->armageddonDamage;

    for (unsigned int i = 0; i < 4; ++i)
        this->pTurnBasedPlayerRecoveryTimes[i] =
            party->pTurnBasedPlayerRecoveryTimes[i];

    for (unsigned int i = 0; i < 53; ++i)
        this->InTheShopFlags[i] = party->InTheShopFlags[i];

    this->uFine = party->uFine;
    this->flt_TorchlightColorR = party->flt_TorchlightColorR;
    this->flt_TorchlightColorG = party->flt_TorchlightColorG;
    this->flt_TorchlightColorB = party->flt_TorchlightColorB;
}

void Party_Image_MM7::Deserialize(Party *party) {
    party->field_0 = this->field_0;
    party->uPartyHeight = this->uPartyHeight;
    party->uDefaultPartyHeight = this->uDefaultPartyHeight;
    party->sEyelevel = this->sEyelevel;
    party->uDefaultEyelevel = this->uDefaultEyelevel;
    party->field_14_radius = this->field_14_radius;
    party->y_rotation_granularity = this->y_rotation_granularity;
    party->uWalkSpeed = this->uWalkSpeed;
    party->y_rotation_speed = this->y_rotation_speed;
    party->field_24 = this->field_24;
    party->field_28 = this->field_28;
    party->playing_time.value = this->uTimePlayed;
    party->last_regenerated.value = this->uLastRegenerationTime;

    for (unsigned int i = 0; i < 10; ++i)
        party->PartyTimes.bountyHunting_next_generation_time[i] =
        GameTime(this->PartyTimes.bountyHunting_next_generation_time[i]);
    for (unsigned int i = 0; i < 85; ++i)
        party->PartyTimes.Shops_next_generation_time[i] =
        GameTime(this->PartyTimes.Shops_next_generation_time[i]);
    for (unsigned int i = 0; i < 53; ++i)
        party->PartyTimes._shop_ban_times[i] =
        GameTime(this->PartyTimes._shop_ban_times[i]);
    for (unsigned int i = 0; i < 10; ++i)
        party->PartyTimes.CounterEventValues[i] =
        GameTime(this->PartyTimes.CounterEventValues[i]);
    for (unsigned int i = 0; i < 29; ++i)
        party->PartyTimes.HistoryEventTimes[i] =
        GameTime(this->PartyTimes.HistoryEventTimes[i]);
    for (unsigned int i = 0; i < 20; ++i)
        party->PartyTimes._s_times[i] = GameTime(this->PartyTimes._s_times[i]);

    party->vPosition.x = this->vPosition.x;
    party->vPosition.y = this->vPosition.y;
    party->vPosition.z = this->vPosition.z;
    party->sRotationZ = this->sRotationZ;
    party->sRotationX = this->sRotationX;
    party->vPrevPosition.x = this->vPrevPosition.x;
    party->vPrevPosition.y = this->vPrevPosition.y;
    party->vPrevPosition.z = this->vPrevPosition.z;
    party->sPrevRotationY = this->sPrevRotationY;
    party->sPrevRotationX = this->sPrevRotationX;
    party->sPrevEyelevel = this->sPrevEyelevel;
    party->field_6E0 = this->field_6E0;
    party->field_6E4 = this->field_6E4;
    party->uFallSpeed = this->uFallSpeed;
    party->field_6EC = this->field_6EC;
    party->field_6F0 = this->field_6F0;
    party->floor_face_pid = this->floor_face_pid;
    party->walk_sound_timer = this->walk_sound_timer;
    party->_6FC_water_lava_timer = this->_6FC_water_lava_timer;
    party->uFallStartY = this->uFallStartY;
    party->bFlying = this->bFlying;
    party->field_708 = this->field_708;
    party->hirelingScrollPosition = this->hirelingScrollPosition;
    party->cNonHireFollowers = this->field_70A;
    party->field_70B = this->field_70B;
    party->uCurrentYear = this->uCurrentYear;
    party->uCurrentMonth = this->uCurrentMonth;
    party->uCurrentMonthWeek = this->uCurrentMonthWeek;
    party->uCurrentDayOfMonth = this->uCurrentDayOfMonth;
    party->uCurrentHour = this->uCurrentHour;
    party->uCurrentMinute = this->uCurrentMinute;
    party->uCurrentTimeSecond = this->uCurrentTimeSecond;
    party->uNumFoodRations = this->uNumFoodRations;
    party->field_72C = this->field_72C;
    party->field_730 = this->field_730;
    party->uNumGold = this->uNumGold;
    party->uNumGoldInBank = this->uNumGoldInBank;
    party->uNumDeaths = this->uNumDeaths;
    party->field_740 = this->field_740;
    party->uNumPrisonTerms = this->uNumPrisonTerms;
    party->uNumBountiesCollected = this->uNumBountiesCollected;
    party->field_74C = this->field_74C;

    for (unsigned int i = 0; i < 5; ++i)
        party->monster_id_for_hunting[i] = this->monster_id_for_hunting[i];
    for (unsigned int i = 0; i < 5; ++i)
        party->monster_for_hunting_killed[i] =
            this->monster_for_hunting_killed[i];

    party->days_played_without_rest = this->days_played_without_rest;

    for (unsigned int i = 0; i < 64; ++i)
        party->_quest_bits[i] = this->_quest_bits[i];
    for (unsigned int i = 0; i < 16; ++i)
        party->pArcomageWins[i] = this->pArcomageWins[i];

    party->field_7B5_in_arena_quest = this->field_7B5_in_arena_quest;
    party->uNumArenaPageWins = this->uNumArenaPageWins;
    party->uNumArenaSquireWins = this->uNumArenaSquireWins;
    party->uNumArenaKnightWins = this->uNumArenaKnightWins;
    party->uNumArenaLordWins = this->uNumArenaLordWins;

    for (unsigned int i = 0; i < 29; ++i)
        party->pIsArtifactFound[i] = this->pIsArtifactFound[i];
    for (unsigned int i = 0; i < 39; ++i)
        party->field_7d7[i] = this->field_7d7[i];
    for (unsigned int i = 0; i < 26; ++i)
        party->_autonote_bits[i] = this->_autonote_bits[i];
    for (unsigned int i = 0; i < 60; ++i)
        party->field_818[i] = this->field_818[i];
    for (unsigned int i = 0; i < 32; ++i)
        party->field_854[i] = this->field_854[i];

    party->uNumArcomageWins = this->uNumArcomageWins;
    party->uNumArcomageLoses = this->uNumArcomageLoses;
    party->bTurnBasedModeOn = this->bTurnBasedModeOn;
    party->field_880 = this->field_880;
    party->uFlags2 = this->uFlags2;

    switch (this->alignment) {
        case 0:
            party->alignment = PartyAlignment::PartyAlignment_Good;
            break;
        case 1:
            party->alignment = PartyAlignment::PartyAlignment_Neutral;
            break;
        case 2:
            party->alignment = PartyAlignment::PartyAlignment_Evil;
            break;
        default:
            Assert(false);
    }

    for (unsigned int i = 0; i < 20; ++i)
        this->pPartyBuffs[i].Deserialize(&party->pPartyBuffs[i]);
    for (unsigned int i = 0; i < 4; ++i)
        this->pPlayers[i].Deserialize(&party->pPlayers[i]);
    for (unsigned int i = 0; i < 2; ++i)
        this->pHirelings[i].Deserialize(&party->pHirelings[i]);

    this->pPickedItem.Deserialize(&party->pPickedItem);

    party->uFlags = this->uFlags;

    for (unsigned int i = 0; i < 53; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            this->StandartItemsInShops[i][j].Deserialize(
                &party->StandartItemsInShops[i][j]);

    for (unsigned int i = 0; i < 53; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            this->SpecialItemsInShops[i][j].Deserialize(
                &party->SpecialItemsInShops[i][j]);

    for (unsigned int i = 0; i < 32; ++i)
        for (unsigned int j = 0; j < 12; ++j)
            this->SpellBooksInGuilds[i][j].Deserialize(
                &party->SpellBooksInGuilds[i][j]);

    for (unsigned int i = 0; i < 24; ++i)
        party->field_1605C[i] = this->field_1605C[i];

    strcpy(party->pHireling1Name, this->pHireling1Name);
    strcpy(party->pHireling2Name, this->pHireling2Name);

    party->armageddon_timer = this->armageddon_timer;
    party->armageddonDamage = this->armageddonDamage;

    for (unsigned int i = 0; i < 4; ++i)
        party->pTurnBasedPlayerRecoveryTimes[i] =
            this->pTurnBasedPlayerRecoveryTimes[i];

    for (unsigned int i = 0; i < 53; ++i)
        party->InTheShopFlags[i] = this->InTheShopFlags[i];

    party->uFine = this->uFine;
    party->flt_TorchlightColorR = this->flt_TorchlightColorR;
    party->flt_TorchlightColorG = this->flt_TorchlightColorG;
    party->flt_TorchlightColorB = this->flt_TorchlightColorB;
}

void Player_Image_MM7::Serialize(Player *player) {
    memset(this, 0, sizeof(*this));

    for (unsigned int i = 0; i < 20; ++i)
        this->pConditions[i] = player->conditions_times[i].value;

    this->uExperience = player->uExperience;

    strcpy(this->pName, player->pName);

    this->uSex = player->uSex;
    this->classType = player->classType;
    this->uCurrentFace = player->uCurrentFace;
    this->field_BB = player->field_BB;
    this->uMight = player->uMight;
    this->uMightBonus = player->uMightBonus;
    this->uIntelligence = player->uIntelligence;
    this->uIntelligenceBonus = player->uIntelligenceBonus;
    this->uWillpower = player->uWillpower;
    this->uWillpowerBonus = player->uWillpowerBonus;
    this->uEndurance = player->uEndurance;
    this->uEnduranceBonus = player->uEnduranceBonus;
    this->uSpeed = player->uSpeed;
    this->uSpeedBonus = player->uSpeedBonus;
    this->uAccuracy = player->uAccuracy;
    this->uAccuracyBonus = player->uAccuracyBonus;
    this->uLuck = player->uLuck;
    this->uLuckBonus = player->uLuckBonus;
    this->sACModifier = player->sACModifier;
    this->uLevel = player->uLevel;
    this->sLevelModifier = player->sLevelModifier;
    this->sAgeModifier = player->sAgeModifier;
    this->field_E0 = player->field_E0;
    this->field_E4 = player->field_E4;
    this->field_E8 = player->field_E8;
    this->field_EC = player->field_EC;
    this->field_F0 = player->field_F0;
    this->field_F4 = player->field_F4;
    this->field_F8 = player->field_F8;
    this->field_FC = player->field_FC;
    this->field_100 = player->field_100;
    this->field_104 = player->field_104;

    for (unsigned int i = 0; i < 37; ++i)
        this->pActiveSkills[i] = player->pActiveSkills[i];

    for (unsigned int i = 0; i < 64; ++i)
        this->_achieved_awards_bits[i] = player->_achieved_awards_bits[i];

    for (unsigned int i = 0; i < 99; ++i)
        this->spellbook.bHaveSpell[i] = player->spellbook.bHaveSpell[i];

    this->pure_luck_used = player->pure_luck_used;
    this->pure_speed_used = player->pure_speed_used;
    this->pure_intellect_used = player->pure_intellect_used;
    this->pure_endurance_used = player->pure_endurance_used;
    this->pure_willpower_used = player->pure_willpower_used;
    this->pure_accuracy_used = player->pure_accuracy_used;
    this->pure_might_used = player->pure_might_used;

    for (unsigned int i = 0; i < 138; ++i)
        this->pOwnItems[i].Serialize(&player->pOwnItems[i]);

    for (unsigned int i = 0; i < 126; ++i)
        this->pInventoryMatrix[i] = player->pInventoryMatrix[i];

    this->sResFireBase = player->sResFireBase;
    this->sResAirBase = player->sResAirBase;
    this->sResWaterBase = player->sResWaterBase;
    this->sResEarthBase = player->sResEarthBase;
    this->field_177C = player->field_177C;
    this->sResMagicBase = player->sResMagicBase;
    this->sResSpiritBase = player->sResSpiritBase;
    this->sResMindBase = player->sResMindBase;
    this->sResBodyBase = player->sResBodyBase;
    this->sResLightBase = player->sResLightBase;
    this->sResDarkBase = player->sResDarkBase;
    this->sResFireBonus = player->sResFireBonus;
    this->sResAirBonus = player->sResAirBonus;
    this->sResWaterBonus = player->sResWaterBonus;
    this->sResEarthBonus = player->sResEarthBonus;
    this->field_1792 = player->field_1792;
    this->sResMagicBonus = player->sResMagicBonus;
    this->sResSpiritBonus = player->sResSpiritBonus;
    this->sResMindBonus = player->sResMindBonus;
    this->sResBodyBonus = player->sResBodyBonus;
    this->sResLightBonus = player->sResLightBonus;
    this->sResDarkBonus = player->sResDarkBonus;

    for (unsigned int i = 0; i < 24; ++i)
        this->pPlayerBuffs[i].Serialize(&player->pPlayerBuffs[i]);

    this->uVoiceID = player->uVoiceID;
    this->uPrevVoiceID = player->uPrevVoiceID;
    this->uPrevFace = player->uPrevFace;
    this->field_192C = player->field_192C;
    this->field_1930 = player->field_1930;
    this->uTimeToRecovery = player->uTimeToRecovery;
    this->field_1936 = player->field_1936;
    this->field_1937 = player->field_1937;
    this->uSkillPoints = player->uSkillPoints;
    this->sHealth = player->sHealth;
    this->sMana = player->sMana;
    this->uBirthYear = player->uBirthYear;

    for (unsigned int i = 0; i < 16; ++i)
        this->pEquipment.pIndices[i] = player->pEquipment.pIndices[i];

    for (unsigned int i = 0; i < 49; ++i)
        this->field_1988[i] = player->field_1988[i];

    this->field_1A4C = player->field_1A4C;
    this->field_1A4D = player->field_1A4D;
    this->lastOpenedSpellbookPage = player->lastOpenedSpellbookPage;
    this->uQuickSpell = player->uQuickSpell;

    for (unsigned int i = 0; i < 49; ++i)
        this->playerEventBits[i] = player->playerEventBits[i];

    this->_some_attack_bonus = player->_some_attack_bonus;
    this->field_1A91 = player->field_1A91;
    this->_melee_dmg_bonus = player->_melee_dmg_bonus;
    this->field_1A93 = player->field_1A93;
    this->_ranged_atk_bonus = player->_ranged_atk_bonus;
    this->field_1A95 = player->field_1A95;
    this->_ranged_dmg_bonus = player->_ranged_dmg_bonus;
    this->field_1A97 = player->field_1A97;
    this->uFullHealthBonus = player->uFullHealthBonus;
    this->_health_related = player->_health_related;
    this->uFullManaBonus = player->uFullManaBonus;
    this->_mana_related = player->_mana_related;
    this->expression = player->expression;
    this->uExpressionTimePassed = player->uExpressionTimePassed;
    this->uExpressionTimeLength = player->uExpressionTimeLength;
    this->field_1AA2 = player->field_1AA2;
    this->_expression21_animtime = player->_expression21_animtime;
    this->_expression21_frameset = player->_expression21_frameset;

    for (unsigned int i = 0; i < 5; ++i) {
        if (i >= player->vBeacons.size()) {
            continue;
        }
        this->pInstalledBeacons[i].uBeaconTime =
            player->vBeacons[i].uBeaconTime;
        this->pInstalledBeacons[i].PartyPos_X =
            player->vBeacons[i].PartyPos_X;
        this->pInstalledBeacons[i].PartyPos_Y =
            player->vBeacons[i].PartyPos_Y;
        this->pInstalledBeacons[i].PartyPos_Z =
            player->vBeacons[i].PartyPos_Z;
        this->pInstalledBeacons[i].PartyRot_X =
            player->vBeacons[i].PartyRot_X;
        this->pInstalledBeacons[i].PartyRot_Y =
            player->vBeacons[i].PartyRot_Y;
        this->pInstalledBeacons[i].SaveFileID =
            player->vBeacons[i].SaveFileID;
    }

    this->uNumDivineInterventionCastsThisDay =
        player->uNumDivineInterventionCastsThisDay;
    this->uNumArmageddonCasts = player->uNumArmageddonCasts;
    this->uNumFireSpikeCasts = player->uNumFireSpikeCasts;
    this->field_1B3B = player->field_1B3B;
}

void Player_Image_MM7::Deserialize(Player* player) {
    for (unsigned int i = 0; i < 20; ++i)
        player->conditions_times[i].value = this->pConditions[i];

    player->uExperience = this->uExperience;

    strcpy(player->pName, this->pName);

    switch (this->uSex) {
    case 0:
        player->uSex = SEX_MALE;
        break;
    case 1:
        player->uSex = SEX_FEMALE;
        break;
    default:
        Assert(false);
    }

    switch (this->classType) {
    case 0:
        player->classType = PLAYER_CLASS_KNIGHT;
        break;
    case 1:
        player->classType = PLAYER_CLASS_CHEVALIER;
        break;
    case 2:
        player->classType = PLAYER_CLASS_CHAMPION;
        break;
    case 3:
        player->classType = PLAYER_CLASS_BLACK_KNIGHT;
        break;
    case 4:
        player->classType = PLAYER_CLASS_THIEF;
        break;
    case 5:
        player->classType = PLAYER_CLASS_ROGUE;
        break;
    case 6:
        player->classType = PLAYER_CLASS_SPY;
        break;
    case 7:
        player->classType = PLAYER_CLASS_ASSASSIN;
        break;
    case 8:
        player->classType = PLAYER_CLASS_MONK;
        break;
    case 9:
        player->classType = PLAYER_CLASS_INITIATE;
        break;
    case 10:
        player->classType = PLAYER_CLASS_MASTER;
        break;
    case 11:
        player->classType = PLAYER_CLASS_NINJA;
        break;
    case 12:
        player->classType = PLAYER_CLASS_PALADIN;
        break;
    case 13:
        player->classType = PLAYER_CLASS_CRUSADER;
        break;
    case 14:
        player->classType = PLAYER_CLASS_HERO;
        break;
    case 15:
        player->classType = PLAYER_CLASS_VILLIAN;
        break;
    case 16:
        player->classType = PLAYER_CLASS_ARCHER;
        break;
    case 17:
        player->classType = PLAYER_CLASS_WARRIOR_MAGE;
        break;
    case 18:
        player->classType = PLAYER_CLASS_MASTER_ARCHER;
        break;
    case 19:
        player->classType = PLAYER_CLASS_SNIPER;
        break;
    case 20:
        player->classType = PLAYER_CLASS_RANGER;
        break;
    case 21:
        player->classType = PLAYER_CLASS_HUNTER;
        break;
    case 22:
        player->classType = PLAYER_CLASS_RANGER_LORD;
        break;
    case 23:
        player->classType = PLAYER_CLASS_BOUNTY_HUNTER;
        break;
    case 24:
        player->classType = PLAYER_CLASS_CLERIC;
        break;
    case 25:
        player->classType = PLAYER_CLASS_PRIEST;
        break;
    case 26:
        player->classType = PLAYER_CLASS_PRIEST_OF_SUN;
        break;
    case 27:
        player->classType = PLAYER_CLASS_PRIEST_OF_MOON;
        break;
    case 28:
        player->classType = PLAYER_CLASS_DRUID;
        break;
    case 29:
        player->classType = PLAYER_CLASS_GREAT_DRUID;
        break;
    case 30:
        player->classType = PLAYER_CLASS_ARCH_DRUID;
        break;
    case 31:
        player->classType = PLAYER_CLASS_WARLOCK;
        break;
    case 32:
        player->classType = PLAYER_CLASS_SORCERER;
        break;
    case 33:
        player->classType = PLAYER_CLASS_WIZARD;
        break;
    case 34:
        player->classType = PLAYER_CLASS_ARCHMAGE;
        break;
    case 35:
        player->classType = PLAYER_CLASS_LICH;
        break;
    default:
        Assert(false);
    }

    player->uCurrentFace = this->uCurrentFace;
    player->field_BB = this->field_BB;
    player->uMight = this->uMight;
    player->uMightBonus = this->uMightBonus;
    player->uIntelligence = this->uIntelligence;
    player->uIntelligenceBonus = this->uIntelligenceBonus;
    player->uWillpower = this->uWillpower;
    player->uWillpowerBonus = this->uWillpowerBonus;
    player->uEndurance = this->uEndurance;
    player->uEnduranceBonus = this->uEnduranceBonus;
    player->uSpeed = this->uSpeed;
    player->uSpeedBonus = this->uSpeedBonus;
    player->uAccuracy = this->uAccuracy;
    player->uAccuracyBonus = this->uAccuracyBonus;
    player->uLuck = this->uLuck;
    player->uLuckBonus = this->uLuckBonus;
    player->sACModifier = this->sACModifier;
    player->uLevel = this->uLevel;
    player->sLevelModifier = this->sLevelModifier;
    player->sAgeModifier = this->sAgeModifier;
    player->field_E0 = this->field_E0;
    player->field_E4 = this->field_E4;
    player->field_E8 = this->field_E8;
    player->field_EC = this->field_EC;
    player->field_F0 = this->field_F0;
    player->field_F4 = this->field_F4;
    player->field_F8 = this->field_F8;
    player->field_FC = this->field_FC;
    player->field_100 = this->field_100;
    player->field_104 = this->field_104;

    for (unsigned int i = 0; i < 37; ++i)
        player->pActiveSkills[i] = this->pActiveSkills[i];

    for (unsigned int i = 0; i < 64; ++i)
        player->_achieved_awards_bits[i] = this->_achieved_awards_bits[i];

    for (unsigned int i = 0; i < 99; ++i)
        player->spellbook.bHaveSpell[i] = this->spellbook.bHaveSpell[i];

    player->pure_luck_used = this->pure_luck_used;
    player->pure_speed_used = this->pure_speed_used;
    player->pure_intellect_used = this->pure_intellect_used;
    player->pure_endurance_used = this->pure_endurance_used;
    player->pure_willpower_used = this->pure_willpower_used;
    player->pure_accuracy_used = this->pure_accuracy_used;
    player->pure_might_used = this->pure_might_used;

    for (unsigned int i = 0; i < 138; ++i)
        this->pOwnItems[i].Deserialize(&player->pOwnItems[i]);

    for (unsigned int i = 0; i < 126; ++i)
        player->pInventoryMatrix[i] = this->pInventoryMatrix[i];

    player->sResFireBase = this->sResFireBase;
    player->sResAirBase = this->sResAirBase;
    player->sResWaterBase = this->sResWaterBase;
    player->sResEarthBase = this->sResEarthBase;
    player->field_177C = this->field_177C;
    player->sResMagicBase = this->sResMagicBase;
    player->sResSpiritBase = this->sResSpiritBase;
    player->sResMindBase = this->sResMindBase;
    player->sResBodyBase = this->sResBodyBase;
    player->sResLightBase = this->sResLightBase;
    player->sResDarkBase = this->sResDarkBase;
    player->sResFireBonus = this->sResFireBonus;
    player->sResAirBonus = this->sResAirBonus;
    player->sResWaterBonus = this->sResWaterBonus;
    player->sResEarthBonus = this->sResEarthBonus;
    player->field_1792 = this->field_1792;
    player->sResMagicBonus = this->sResMagicBonus;
    player->sResSpiritBonus = this->sResSpiritBonus;
    player->sResMindBonus = this->sResMindBonus;
    player->sResBodyBonus = this->sResBodyBonus;
    player->sResLightBonus = this->sResLightBonus;
    player->sResDarkBonus = this->sResDarkBonus;

    for (unsigned int i = 0; i < 24; ++i)
        this->pPlayerBuffs[i].Deserialize(&player->pPlayerBuffs[i]);

    player->uVoiceID = this->uVoiceID;
    player->uPrevVoiceID = this->uPrevVoiceID;
    player->uPrevFace = this->uPrevFace;
    player->field_192C = this->field_192C;
    player->field_1930 = this->field_1930;
    player->uTimeToRecovery = this->uTimeToRecovery;
    player->field_1936 = this->field_1936;
    player->field_1937 = this->field_1937;
    player->uSkillPoints = this->uSkillPoints;
    player->sHealth = this->sHealth;
    player->sMana = this->sMana;
    player->uBirthYear = this->uBirthYear;

    for (unsigned int i = 0; i < 16; ++i)
        player->pEquipment.pIndices[i] = this->pEquipment.pIndices[i];

    for (unsigned int i = 0; i < 49; ++i)
        player->field_1988[i] = this->field_1988[i];

    player->field_1A4C = this->field_1A4C;
    player->field_1A4D = this->field_1A4D;
    player->lastOpenedSpellbookPage = this->lastOpenedSpellbookPage;
    player->uQuickSpell = this->uQuickSpell;

    for (unsigned int i = 0; i < 49; ++i)
        player->playerEventBits[i] = this->playerEventBits[i];

    player->_some_attack_bonus = this->_some_attack_bonus;
    player->field_1A91 = this->field_1A91;
    player->_melee_dmg_bonus = this->_melee_dmg_bonus;
    player->field_1A93 = this->field_1A93;
    player->_ranged_atk_bonus = this->_ranged_atk_bonus;
    player->field_1A95 = this->field_1A95;
    player->_ranged_dmg_bonus = this->_ranged_dmg_bonus;
    player->field_1A97 = this->field_1A97;
    player->uFullHealthBonus = this->uFullHealthBonus;
    player->_health_related = this->_health_related;
    player->uFullManaBonus = this->uFullManaBonus;
    player->_mana_related = this->_mana_related;
    player->expression = (CHARACTER_EXPRESSION_ID)this->expression;
    player->uExpressionTimePassed = this->uExpressionTimePassed;
    player->uExpressionTimeLength = this->uExpressionTimeLength;
    player->field_1AA2 = this->field_1AA2;
    player->_expression21_animtime = this->_expression21_animtime;
    player->_expression21_frameset = this->_expression21_frameset;

    for (int z = 0; z < player->vBeacons.size(); z++) {
        player->vBeacons[z].image->Release();
    }
    player->vBeacons.clear();

    for (unsigned int i = 0; i < 5; ++i) {
        if (this->pInstalledBeacons[i].uBeaconTime != 0) {
            LloydBeacon beacon;
            beacon.uBeaconTime = GameTime(this->pInstalledBeacons[i].uBeaconTime);
            beacon.PartyPos_X = this->pInstalledBeacons[i].PartyPos_X;
            beacon.PartyPos_Y = this->pInstalledBeacons[i].PartyPos_Y;
            beacon.PartyPos_Z = this->pInstalledBeacons[i].PartyPos_Z;
            beacon.PartyRot_X = this->pInstalledBeacons[i].PartyRot_X;
            beacon.PartyRot_Y = this->pInstalledBeacons[i].PartyRot_Y;
            beacon.SaveFileID = this->pInstalledBeacons[i].SaveFileID;
            player->vBeacons.push_back(beacon);
        }
    }

    player->uNumDivineInterventionCastsThisDay =
        this->uNumDivineInterventionCastsThisDay;
    player->uNumArmageddonCasts = this->uNumArmageddonCasts;
    player->uNumFireSpikeCasts = this->uNumFireSpikeCasts;
    player->field_1B3B = this->field_1B3B;
}

void IconFrame_MM7::Serialize(Icon *icon) {
    strcpy(pAnimationName, icon->GetAnimationName());
    uAnimLength = icon->GetAnimLength();

    strcpy(pTextureName, icon->pTextureName);
    uAnimTime = icon->GetAnimTime();
    uFlags = icon->uFlags;
}

void IconFrame_MM7::Deserialize(Icon *icon) {
    icon->SetAnimationName(this->pAnimationName);
    icon->SetAnimLength(8 * this->uAnimLength);

    strcpy(icon->pTextureName, pTextureName);
    icon->SetAnimTime(uAnimTime);
    icon->uFlags = uFlags;
}

void UIAnimation_MM7::Serialize(UIAnimation *anim) {
    /* 000 */ uIconID = anim->icon->id;
    /* 002 */ field_2 = anim->field_2;
    /* 004 */ uAnimTime = anim->uAnimTime;
    /* 006 */ uAnimLength = anim->uAnimLength;
    /* 008 */ x = anim->x;
    /* 00A */ y = anim->y;
    /* 00C */ field_C = anim->field_C;
}

void UIAnimation_MM7::Deserialize(UIAnimation *anim) {
    anim->icon = pIconsFrameTable->GetIcon(uIconID);
    ///* 000 */ anim->uIconID = uIconID;
    /* 002 */ anim->field_2 = field_2;
    /* 004 */ anim->uAnimTime = uAnimTime;
    /* 006 */ anim->uAnimLength = uAnimLength;
    /* 008 */ anim->x = x;
    /* 00A */ anim->y = y;
    /* 00C */ anim->field_C = field_C;
}

void Actor_MM7::Serialize(Actor *actor) {
    for (unsigned int i = 0; i < 32; ++i)
        this->pActorName[i] = actor->pActorName[i];

    this->sNPC_ID = actor->sNPC_ID;
    this->field_22 = actor->field_22;
    this->uAttributes = actor->uAttributes;
    this->sCurrentHP = actor->sCurrentHP;

    for (unsigned int i = 0; i < 2; ++i)
        this->field_2A[i] = actor->field_2A[i];

    this->pMonsterInfo.uLevel = actor->pMonsterInfo.uLevel;
    this->pMonsterInfo.uTreasureDropChance = actor->pMonsterInfo.uTreasureDropChance;
    this->pMonsterInfo.uTreasureDiceRolls = actor->pMonsterInfo.uTreasureDiceRolls;
    this->pMonsterInfo.uTreasureDiceSides = actor->pMonsterInfo.uTreasureDiceSides;
    this->pMonsterInfo.uTreasureLevel = actor->pMonsterInfo.uTreasureLevel;
    this->pMonsterInfo.uTreasureType = actor->pMonsterInfo.uTreasureType;
    this->pMonsterInfo.uFlying = actor->pMonsterInfo.uFlying;
    this->pMonsterInfo.uMovementType = actor->pMonsterInfo.uMovementType;
    this->pMonsterInfo.uAIType = actor->pMonsterInfo.uAIType;
    this->pMonsterInfo.uHostilityType = (uint8_t)actor->pMonsterInfo.uHostilityType;
    this->pMonsterInfo.field_12 = actor->pMonsterInfo.field_12;
    this->pMonsterInfo.uSpecialAttackType = actor->pMonsterInfo.uSpecialAttackType;
    this->pMonsterInfo.uSpecialAttackLevel = actor->pMonsterInfo.uSpecialAttackLevel;
    this->pMonsterInfo.uAttack1Type = actor->pMonsterInfo.uAttack1Type;
    this->pMonsterInfo.uAttack1DamageDiceRolls = actor->pMonsterInfo.uAttack1DamageDiceRolls;
    this->pMonsterInfo.uAttack1DamageDiceSides = actor->pMonsterInfo.uAttack1DamageDiceSides;
    this->pMonsterInfo.uAttack1DamageBonus = actor->pMonsterInfo.uAttack1DamageBonus;
    this->pMonsterInfo.uMissleAttack1Type = actor->pMonsterInfo.uMissleAttack1Type;
    this->pMonsterInfo.uAttack2Chance = actor->pMonsterInfo.uAttack2Chance;
    this->pMonsterInfo.uAttack2Type = actor->pMonsterInfo.uAttack2Type;
    this->pMonsterInfo.uAttack2DamageDiceRolls = actor->pMonsterInfo.uAttack2DamageDiceRolls;
    this->pMonsterInfo.uAttack2DamageDiceSides = actor->pMonsterInfo.uAttack2DamageDiceSides;
    this->pMonsterInfo.uAttack2DamageBonus = actor->pMonsterInfo.uAttack2DamageBonus;
    this->pMonsterInfo.uMissleAttack2Type = actor->pMonsterInfo.uMissleAttack2Type;
    this->pMonsterInfo.uSpell1UseChance = actor->pMonsterInfo.uSpell1UseChance;
    this->pMonsterInfo.uSpell1ID = actor->pMonsterInfo.uSpell1ID;
    this->pMonsterInfo.uSpell2UseChance = actor->pMonsterInfo.uSpell2UseChance;
    this->pMonsterInfo.uSpell2ID = actor->pMonsterInfo.uSpell2ID;
    this->pMonsterInfo.uResFire = actor->pMonsterInfo.uResFire;
    this->pMonsterInfo.uResAir = actor->pMonsterInfo.uResAir;
    this->pMonsterInfo.uResWater = actor->pMonsterInfo.uResWater;
    this->pMonsterInfo.uResEarth = actor->pMonsterInfo.uResEarth;
    this->pMonsterInfo.uResMind = actor->pMonsterInfo.uResMind;
    this->pMonsterInfo.uResSpirit = actor->pMonsterInfo.uResSpirit;
    this->pMonsterInfo.uResBody = actor->pMonsterInfo.uResBody;
    this->pMonsterInfo.uResLight = actor->pMonsterInfo.uResLight;
    this->pMonsterInfo.uResDark = actor->pMonsterInfo.uResDark;
    this->pMonsterInfo.uResPhysical = actor->pMonsterInfo.uResPhysical;
    this->pMonsterInfo.uSpecialAbilityType = actor->pMonsterInfo.uSpecialAbilityType;
    this->pMonsterInfo.uSpecialAbilityDamageDiceRolls = actor->pMonsterInfo.uSpecialAbilityDamageDiceRolls;
    this->pMonsterInfo.uSpecialAbilityDamageDiceSides = actor->pMonsterInfo.uSpecialAbilityDamageDiceSides;
    this->pMonsterInfo.uSpecialAbilityDamageDiceBonus = actor->pMonsterInfo.uSpecialAbilityDamageDiceBonus;
    this->pMonsterInfo.uNumCharactersAttackedPerSpecialAbility = actor->pMonsterInfo.uNumCharactersAttackedPerSpecialAbility;
    this->pMonsterInfo.field_33 = actor->pMonsterInfo.field_33;
    this->pMonsterInfo.uID = actor->pMonsterInfo.uID;
    this->pMonsterInfo.bQuestMonster = actor->pMonsterInfo.bQuestMonster;
    this->pMonsterInfo.uSpellSkillAndMastery1 = actor->pMonsterInfo.uSpellSkillAndMastery1;
    this->pMonsterInfo.uSpellSkillAndMastery2 = actor->pMonsterInfo.uSpellSkillAndMastery2;
    this->pMonsterInfo.field_3C_some_special_attack = actor->pMonsterInfo.field_3C_some_special_attack;
    this->pMonsterInfo.field_3E = actor->pMonsterInfo.field_3E;
    this->pMonsterInfo.uHP = actor->pMonsterInfo.uHP;
    this->pMonsterInfo.uAC = actor->pMonsterInfo.uAC;
    this->pMonsterInfo.uExp = actor->pMonsterInfo.uExp;
    this->pMonsterInfo.uBaseSpeed = actor->pMonsterInfo.uBaseSpeed;
    this->pMonsterInfo.uRecoveryTime = actor->pMonsterInfo.uRecoveryTime;
    this->pMonsterInfo.uAttackPreference = actor->pMonsterInfo.uAttackPreference;
    this->word_000084_range_attack = actor->word_000084_range_attack;
    this->word_000086_some_monster_id = actor->word_000086_some_monster_id;  // base monster class monsterlist id
    this->uActorRadius = actor->uActorRadius;
    this->uActorHeight = actor->uActorHeight;
    this->uMovementSpeed = actor->uMovementSpeed;
    this->vPosition = actor->vPosition;
    this->vVelocity = actor->vVelocity;
    this->uYawAngle = actor->uYawAngle;
    this->uPitchAngle = actor->uPitchAngle;
    this->uSectorID = actor->uSectorID;
    this->uCurrentActionLength = actor->uCurrentActionLength;
    this->vInitialPosition = actor->vInitialPosition;
    this->vGuardingPosition = actor->vGuardingPosition;
    this->uTetherDistance = actor->uTetherDistance;
    this->uAIState = actor->uAIState;
    this->uCurrentActionAnimation = actor->uCurrentActionAnimation;
    this->uCarriedItemID = actor->uCarriedItemID;
    this->field_B6 = actor->field_B6;
    this->field_B7 = actor->field_B7;
    this->uCurrentActionTime = actor->uCurrentActionTime;

    for (unsigned int i = 0; i < 8; ++i)
        this->pSpriteIDs[i] = actor->pSpriteIDs[i];

    for (unsigned int i = 0; i < 4; ++i)
        this->pSoundSampleIDs[i] = actor->pSoundSampleIDs[i];

    for (unsigned int i = 0; i < 22; ++i)
        this->pActorBuffs[i] = actor->pActorBuffs[i];

    for (unsigned int i = 0; i < 4; ++i)
        this->ActorHasItems[i] = actor->ActorHasItems[i];

    this->uGroup = actor->uGroup;
    this->uAlly = actor->uAlly;

    for (unsigned int i = 0; i < 8; ++i)
        this->pScheduledJobs[i] = actor->pScheduledJobs[i];

    this->uSummonerID = actor->uSummonerID;
    this->uLastCharacterIDToHit = actor->uLastCharacterIDToHit;
    this->dword_000334_unique_name = actor->dword_000334_unique_name;

    for (unsigned int i = 0; i < 12; ++i)
        this->field_338[i] = actor->field_338[i];
}

void Actor_MM7::Deserialize(Actor *actor) {
    for (unsigned int i = 0; i < 32; ++i)
        actor->pActorName[i] = this->pActorName[i];

    actor->sNPC_ID = this->sNPC_ID;
    actor->field_22 = this->field_22;
    actor->uAttributes = this->uAttributes;
    actor->sCurrentHP = this->sCurrentHP;

    for (unsigned int i = 0; i < 2; ++i)
        actor->field_2A[i] = this->field_2A[i];

    actor->pMonsterInfo.uLevel = this->pMonsterInfo.uLevel;
    actor->pMonsterInfo.uTreasureDropChance = this->pMonsterInfo.uTreasureDropChance;
    actor->pMonsterInfo.uTreasureDiceRolls = this->pMonsterInfo.uTreasureDiceRolls;
    actor->pMonsterInfo.uTreasureDiceSides = this->pMonsterInfo.uTreasureDiceSides;
    actor->pMonsterInfo.uTreasureLevel = this->pMonsterInfo.uTreasureLevel;
    actor->pMonsterInfo.uTreasureType = this->pMonsterInfo.uTreasureType;
    actor->pMonsterInfo.uFlying = this->pMonsterInfo.uFlying;
    actor->pMonsterInfo.uMovementType = this->pMonsterInfo.uMovementType;
    actor->pMonsterInfo.uAIType = this->pMonsterInfo.uAIType;
    actor->pMonsterInfo.uHostilityType = (MonsterInfo::HostilityRadius)this->pMonsterInfo.uHostilityType;
    actor->pMonsterInfo.field_12 = this->pMonsterInfo.field_12;
    actor->pMonsterInfo.uSpecialAttackType = (SPECIAL_ATTACK_TYPE)this->pMonsterInfo.uSpecialAttackType;
    actor->pMonsterInfo.uSpecialAttackLevel = this->pMonsterInfo.uSpecialAttackLevel;
    actor->pMonsterInfo.uAttack1Type = this->pMonsterInfo.uAttack1Type;
    actor->pMonsterInfo.uAttack1DamageDiceRolls = this->pMonsterInfo.uAttack1DamageDiceRolls;
    actor->pMonsterInfo.uAttack1DamageDiceSides = this->pMonsterInfo.uAttack1DamageDiceSides;
    actor->pMonsterInfo.uAttack1DamageBonus = this->pMonsterInfo.uAttack1DamageBonus;
    actor->pMonsterInfo.uMissleAttack1Type = this->pMonsterInfo.uMissleAttack1Type;
    actor->pMonsterInfo.uAttack2Chance = this->pMonsterInfo.uAttack2Chance;
    actor->pMonsterInfo.uAttack2Type = this->pMonsterInfo.uAttack2Type;
    actor->pMonsterInfo.uAttack2DamageDiceRolls = this->pMonsterInfo.uAttack2DamageDiceRolls;
    actor->pMonsterInfo.uAttack2DamageDiceSides = this->pMonsterInfo.uAttack2DamageDiceSides;
    actor->pMonsterInfo.uAttack2DamageBonus = this->pMonsterInfo.uAttack2DamageBonus;
    actor->pMonsterInfo.uMissleAttack2Type = this->pMonsterInfo.uMissleAttack2Type;
    actor->pMonsterInfo.uSpell1UseChance = this->pMonsterInfo.uSpell1UseChance;
    actor->pMonsterInfo.uSpell1ID = this->pMonsterInfo.uSpell1ID;
    actor->pMonsterInfo.uSpell2UseChance = this->pMonsterInfo.uSpell2UseChance;
    actor->pMonsterInfo.uSpell2ID = this->pMonsterInfo.uSpell2ID;
    actor->pMonsterInfo.uResFire = this->pMonsterInfo.uResFire;
    actor->pMonsterInfo.uResAir = this->pMonsterInfo.uResAir;
    actor->pMonsterInfo.uResWater = this->pMonsterInfo.uResWater;
    actor->pMonsterInfo.uResEarth = this->pMonsterInfo.uResEarth;
    actor->pMonsterInfo.uResMind = this->pMonsterInfo.uResMind;
    actor->pMonsterInfo.uResSpirit = this->pMonsterInfo.uResSpirit;
    actor->pMonsterInfo.uResBody = this->pMonsterInfo.uResBody;
    actor->pMonsterInfo.uResLight = this->pMonsterInfo.uResLight;
    actor->pMonsterInfo.uResDark = this->pMonsterInfo.uResDark;
    actor->pMonsterInfo.uResPhysical = this->pMonsterInfo.uResPhysical;
    actor->pMonsterInfo.uSpecialAbilityType = this->pMonsterInfo.uSpecialAbilityType;
    actor->pMonsterInfo.uSpecialAbilityDamageDiceRolls = this->pMonsterInfo.uSpecialAbilityDamageDiceRolls;
    actor->pMonsterInfo.uSpecialAbilityDamageDiceSides = this->pMonsterInfo.uSpecialAbilityDamageDiceSides;
    actor->pMonsterInfo.uSpecialAbilityDamageDiceBonus = this->pMonsterInfo.uSpecialAbilityDamageDiceBonus;
    actor->pMonsterInfo.uNumCharactersAttackedPerSpecialAbility = this->pMonsterInfo.uNumCharactersAttackedPerSpecialAbility;
    actor->pMonsterInfo.field_33 = this->pMonsterInfo.field_33;
    actor->pMonsterInfo.uID = this->pMonsterInfo.uID;
    actor->pMonsterInfo.bQuestMonster = this->pMonsterInfo.bQuestMonster;
    actor->pMonsterInfo.uSpellSkillAndMastery1 = this->pMonsterInfo.uSpellSkillAndMastery1;
    actor->pMonsterInfo.uSpellSkillAndMastery2 = this->pMonsterInfo.uSpellSkillAndMastery2;
    actor->pMonsterInfo.field_3C_some_special_attack = this->pMonsterInfo.field_3C_some_special_attack;
    actor->pMonsterInfo.field_3E = this->pMonsterInfo.field_3E;
    actor->pMonsterInfo.uHP = this->pMonsterInfo.uHP;
    actor->pMonsterInfo.uAC = this->pMonsterInfo.uAC;
    actor->pMonsterInfo.uExp = this->pMonsterInfo.uExp;
    actor->pMonsterInfo.uBaseSpeed = this->pMonsterInfo.uBaseSpeed;
    actor->pMonsterInfo.uRecoveryTime = this->pMonsterInfo.uRecoveryTime;
    actor->pMonsterInfo.uAttackPreference = this->pMonsterInfo.uAttackPreference;
    actor->word_000084_range_attack = this->word_000084_range_attack;
    actor->word_000086_some_monster_id = this->word_000086_some_monster_id;  // base monster class monsterlist id
    actor->uActorRadius = this->uActorRadius;
    actor->uActorHeight = this->uActorHeight;
    actor->uMovementSpeed = this->uMovementSpeed;
    actor->vPosition = this->vPosition;
    actor->vVelocity = this->vVelocity;
    actor->uYawAngle = this->uYawAngle;
    actor->uPitchAngle = this->uPitchAngle;
    actor->uSectorID = this->uSectorID;
    actor->uCurrentActionLength = this->uCurrentActionLength;
    actor->vInitialPosition = this->vInitialPosition;
    actor->vGuardingPosition = this->vGuardingPosition;
    actor->uTetherDistance = this->uTetherDistance;
    actor->uAIState = (AIState)this->uAIState;
    actor->uCurrentActionAnimation = this->uCurrentActionAnimation;
    actor->uCarriedItemID = this->uCarriedItemID;
    actor->field_B6 = this->field_B6;
    actor->field_B7 = this->field_B7;
    actor->uCurrentActionTime = this->uCurrentActionTime;

    for (unsigned int i = 0; i < 8; ++i)
        actor->pSpriteIDs[i] = this->pSpriteIDs[i];

    for (unsigned int i = 0; i < 4; ++i)
        actor->pSoundSampleIDs[i] = this->pSoundSampleIDs[i];

    for (unsigned int i = 0; i < 22; ++i)
        actor->pActorBuffs[i] = this->pActorBuffs[i];

    for (unsigned int i = 0; i < 4; ++i)
        actor->ActorHasItems[i] = this->ActorHasItems[i];

    actor->uGroup = this->uGroup;
    actor->uAlly = this->uAlly;

    for (unsigned int i = 0; i < 8; ++i)
        actor->pScheduledJobs[i] = this->pScheduledJobs[i];

    actor->uSummonerID = this->uSummonerID;
    actor->uLastCharacterIDToHit = this->uLastCharacterIDToHit;
    actor->dword_000334_unique_name = this->dword_000334_unique_name;

    for (unsigned int i = 0; i < 12; ++i)
        actor->field_338[i] = this->field_338[i];
}

void BLVSector_MM7::Serialize(BLVSector *sector) {
    this->field_0 = sector->field_0;
    this->uNumFloors = sector->uNumFloors;
    this->field_6 = sector->field_6;
    this->uNumWalls = sector->uNumWalls;
    this->field_E = sector->field_E;
    this->uNumCeilings = sector->uNumCeilings;
    this->field_16 = sector->field_16;
    this->uNumFluids = sector->uNumFluids;
    this->field_1E = sector->field_1E;
    this->uNumPortals = sector->uNumPortals;
    this->field_26 = sector->field_26;
    this->uNumFaces = sector->uNumFaces;
    this->uNumNonBSPFaces = sector->uNumNonBSPFaces;
    this->uNumCylinderFaces = sector->uNumCylinderFaces;
    this->field_36 = sector->field_36;
    this->pCylinderFaces = sector->pCylinderFaces;
    this->uNumCogs = sector->uNumCogs;
    this->field_3E = sector->field_3E;
    this->uNumDecorations = sector->uNumDecorations;
    this->field_46 = sector->field_46;
    this->uNumMarkers = sector->uNumMarkers;
    this->field_4E = sector->field_4E;
    this->uNumLights = sector->uNumLights;
    this->field_56 = sector->field_56;
    this->uWaterLevel = sector->uWaterLevel;
    this->uMistLevel = sector->uMistLevel;
    this->uLightDistanceMultiplier = sector->uLightDistanceMultiplier;
    this->uMinAmbientLightLevel = sector->uMinAmbientLightLevel;
    this->uFirstBSPNode = sector->uFirstBSPNode;
    this->exit_tag = sector->exit_tag;
    this->pBounding = sector->pBounding;
}

void BLVSector_MM7::Deserialize(BLVSector *sector) {
    sector->field_0 = this->field_0;
    sector->uNumFloors = this->uNumFloors;
    sector->field_6 = this->field_6;
    sector->uNumWalls = this->uNumWalls;
    sector->field_E = this->field_E;
    sector->uNumCeilings = this->uNumCeilings;
    sector->field_16 = this->field_16;
    sector->uNumFluids = this->uNumFluids;
    sector->field_1E = this->field_1E;
    sector->uNumPortals = this->uNumPortals;
    sector->field_26 = this->field_26;
    sector->uNumFaces = this->uNumFaces;
    sector->uNumNonBSPFaces = this->uNumNonBSPFaces;
    sector->uNumCylinderFaces = this->uNumCylinderFaces;
    sector->field_36 = this->field_36;
    sector->pCylinderFaces = this->pCylinderFaces;
    sector->uNumCogs = this->uNumCogs;
    sector->field_3E = this->field_3E;
    sector->uNumDecorations = this->uNumDecorations;
    sector->field_46 = this->field_46;
    sector->uNumMarkers = this->uNumMarkers;
    sector->field_4E = this->field_4E;
    sector->uNumLights = this->uNumLights;
    sector->field_56 = this->field_56;
    sector->uWaterLevel = this->uWaterLevel;
    sector->uMistLevel = this->uMistLevel;
    sector->uLightDistanceMultiplier = this->uLightDistanceMultiplier;
    sector->uMinAmbientLightLevel = this->uMinAmbientLightLevel;
    sector->uFirstBSPNode = this->uFirstBSPNode;
    sector->exit_tag = this->exit_tag;
    sector->pBounding = this->pBounding;
}

void FontData_MM7::Serialize(FontData *font) {
    this->cFirstChar = font->cFirstChar;
    this->cLastChar = font->cLastChar;
    this->field_2 = font->field_2;
    this->field_3 = font->field_3;
    this->field_4 = font->field_4;
    this->uFontHeight = font->uFontHeight;
    this->field_7 = font->field_7;
    this->palletes_count = font->palletes_count;

    for (unsigned int i = 0; i < 256; ++i)
        this->pMetrics[i] = font->pMetrics[i];

    for (unsigned int i = 0; i < 256; ++i)
        this->font_pixels_offset[i] = font->font_pixels_offset[i];

    std::copy(font->pFontData.begin(), font->pFontData.end(), this->pFontData);
}

void FontData_MM7::Deserialize(FontData *font, size_t size) {
    font->cFirstChar = this->cFirstChar;
    font->cLastChar = this->cLastChar;
    font->field_2 = this->field_2;
    font->field_3 = this->field_3;
    font->field_4 = this->field_4;
    font->uFontHeight = this->uFontHeight;
    font->field_7 = this->field_7;
    font->palletes_count = this->palletes_count;

    for (unsigned int i = 0; i < 256; ++i)
        font->pMetrics[i] = this->pMetrics[i];

    for (unsigned int i = 0; i < 256; ++i)
        font->font_pixels_offset[i] = this->font_pixels_offset[i];

    font->pFontData.assign(this->pFontData, &this->pFontData[size - 4128]);
}
