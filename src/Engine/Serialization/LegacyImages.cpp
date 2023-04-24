#include "LegacyImages.h"

#include <algorithm>
#include <type_traits>
#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Time.h"

#include "GUI/GUIFont.h"

#include "Utility/Color.h"
#include "Utility/Memory/MemSet.h"

template<class T>
static void Serialize(const T &src, T *dst) {
    *dst = src;
}

template<class T>
static void Deserialize(const T &src, T *dst) {
    *dst = src;
}

static void Serialize(const GameTime &src, int64_t *dst) {
    *dst = src.value;
}

static void Deserialize(int64_t src, GameTime *dst) {
    dst->value = src;
}

template<size_t N>
static void Serialize(const std::string &src, std::array<char, N> *dst) {
    memset(dst->data(), 0, N);
    memcpy(dst->data(), src.data(), std::min(src.size(), N - 1));
}

template<size_t N>
static void Deserialize(const std::array<char, N> &src, std::string *dst) {
    const char *end = static_cast<const char *>(memchr(src.data(), 0, N));
    size_t size = end == nullptr ? N : end - src.data();
    *dst = std::string(src.data(), size);
}

template<class T1, size_t N, class T2, auto L, auto H>
static void Serialize(const IndexedArray<T2, L, H> &src, std::array<T1, N> *dst) {
    static_assert(IndexedArray<T2, L, H>::SIZE == N, "Expected arrays of equal size.");
    for (size_t i = 0; auto index : src.indices())
        Serialize(src[index], &(*dst)[i++]);
}

template<class T1, size_t N, class T2, auto L, auto H>
static void Deserialize(const std::array<T1, N> &src, IndexedArray<T2, L, H> *dst) {
    static_assert(IndexedArray<T2, L, H>::SIZE == N, "Expected arrays of equal size.");
    for (size_t i = 0; auto index : dst->indices())
        Deserialize(src[i++], &(*dst)[index]);
}

template<class T1, size_t N1, class T2, size_t N2> requires (!std::is_same_v<T1, T2>)
static void Serialize(const std::array<T1, N1> &src, std::array<T2, N2> *dst) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        Serialize(src[i], &(*dst)[i]);
}

template<class T1, size_t N1, class T2, size_t N2> requires (!std::is_same_v<T1, T2>)
static void Deserialize(const std::array<T1, N1> &src, std::array<T2, N2> *dst) {
    static_assert(N1 == N2, "Expected arrays of equal size.");
    for (size_t i = 0; i < N1; i++)
        Deserialize(src[i], &(*dst)[i]);
}

template<class T1, size_t N, class T2, auto L, auto H>
static void Serialize(const IndexedArray<T2, L, H> &src, std::array<T1, N> *dst, size_t count) {
    assert((count == src.size() || count == dst->size()) && src.size() != dst->size());
    for (size_t i = 0; i < count; i++)
        Serialize(src[src.indices()[i]], &(*dst)[i]);
}

template<class T1, size_t N, class T2, auto L, auto H>
static void Deserialize(const std::array<T1, N> &src, IndexedArray<T2, L, H> *dst, size_t count) {
    assert((count == src.size() || count == dst->size()) && src.size() != dst->size());
    for (int i = 0; i < count; i++)
        Deserialize(src[i], &(*dst)[dst->indices()[i]]);
}

void Deserialize(const SpriteFrame_MM7 &src, SpriteFrame *dst) {
    dst->icon_name = src.iconName.data();
    std::transform(dst->icon_name.begin(), dst->icon_name.end(), dst->icon_name.begin(), ::tolower);

    dst->texture_name = src.textureName.data();
    std::transform(dst->texture_name.begin(), dst->texture_name.end(), dst->texture_name.begin(), ::tolower);

    for (unsigned int i = 0; i < 8; ++i)
        dst->hw_sprites[i] = nullptr;

    dst->scale = src.scale / 65536.0;
    dst->uFlags = src.flags;

    dst->uGlowRadius = src.glowRadius;
    dst->uPaletteID = src.paletteId;
    dst->ResetPaletteIndex(src.paletteIndex);
    dst->uAnimTime = src.animTime;
    dst->uAnimLength = src.animLength;
}

void Deserialize(const BLVFace_MM7 &src, BLVFace *dst) {
    dst->facePlane = src.facePlane;
    dst->facePlane_old = src.facePlane_old;
    dst->zCalc.init(dst->facePlane_old);
    dst->uAttributes = static_cast<FaceAttributes>(src.attributes);
    dst->pVertexIDs = nullptr;
    dst->pXInterceptDisplacements = nullptr;
    dst->pYInterceptDisplacements = nullptr;
    dst->pZInterceptDisplacements = nullptr;
    dst->pVertexUIDs = nullptr;
    dst->pVertexVIDs = nullptr;
    dst->uFaceExtraID = src.faceExtraId;
    dst->resource = nullptr;
    dst->uSectorID = src.sectorId;
    dst->uBackSectorID = src.backSectorId;
    dst->pBounding = src.bounding;
    dst->uPolygonType = static_cast<PolygonType>(src.polygonType);
    dst->uNumVertices = src.numVertices;
}

void Serialize(const Timer &src, Timer_MM7 *dst) {
    memzero(dst);

    dst->bReady = src.bReady;
    dst->bPaused = src.bPaused;
    dst->bTackGameTime = src.bTackGameTime;
    dst->uStartTime = src.uStartTime;
    dst->uStopTime = src.uStopTime;
    dst->uGameTimeStart = src.uGameTimeStart;
    dst->field_18 = src.field_18;
    dst->uTimeElapsed = src.uTimeElapsed;
    dst->dt_fixpoint = src.dt_fixpoint;
    dst->uTotalGameTimeElapsed = src.uTotalGameTimeElapsed;
}

void Deserialize(const Timer_MM7 &src, Timer *dst) {
    dst->bReady = src.bReady;
    dst->bPaused = src.bPaused;
    dst->bTackGameTime = src.bTackGameTime;
    dst->uStartTime = src.uStartTime;
    dst->uStopTime = src.uStopTime;
    dst->uGameTimeStart = src.uGameTimeStart;
    dst->field_18 = src.field_18;
    dst->uTimeElapsed = src.uTimeElapsed;
    dst->dt_fixpoint = src.dt_fixpoint;
    dst->uTotalGameTimeElapsed = src.uTotalGameTimeElapsed;
}

void Serialize(const NPCData &src, NPCData_MM7 *dst) {
    memzero(dst);

    // dst->pName = src.pName;
    dst->name = !src.pName.empty();
    dst->portraitId = src.uPortraitID;
    dst->flags = src.uFlags;
    dst->fame = src.fame;
    dst->rep = src.rep;
    dst->location2d = src.Location2D;
    dst->profession = std::to_underlying(src.profession);
    dst->greet = src.greet;
    dst->joins = src.is_joinable;
    dst->field_24 = src.field_24;
    dst->evt_A = src.dialogue_1_evt_id;
    dst->evt_B = src.dialogue_2_evt_id;
    dst->evt_C = src.dialogue_3_evt_id;
    dst->evt_D = src.dialogue_4_evt_id;
    dst->evt_E = src.dialogue_5_evt_id;
    dst->evt_F = src.dialogue_6_evt_id;
    dst->sex = src.uSex;
    dst->hasUsedAbility = src.bHasUsedTheAbility;
    dst->newsTopic = src.news_topic;
}

void Deserialize(const NPCData_MM7 &src, NPCData *dst) {
    // dst->pName = src.pName;
    dst->pName = src.name ? "Dummy" : "";
    dst->uPortraitID = src.portraitId;
    dst->uFlags = src.flags;
    dst->fame = src.fame;
    dst->rep = src.rep;
    dst->Location2D = src.location2d;
    dst->profession = static_cast<NPCProf>(src.profession);
    dst->greet = src.greet;
    dst->is_joinable = src.joins;
    dst->field_24 = src.field_24;
    dst->dialogue_1_evt_id = src.evt_A;
    dst->dialogue_2_evt_id = src.evt_B;
    dst->dialogue_3_evt_id = src.evt_C;
    dst->dialogue_4_evt_id = src.evt_D;
    dst->dialogue_5_evt_id = src.evt_E;
    dst->dialogue_6_evt_id = src.evt_F;
    dst->uSex = src.sex;
    dst->bHasUsedTheAbility = src.hasUsedAbility;
    dst->news_topic = src.newsTopic;
}

void Serialize(const OtherOverlay &src, OtherOverlay_MM7 *dst) {
    memzero(dst);

    dst->field_0 = src.field_0;
    dst->field_2 = src.field_2;
    dst->sprite_frame_time = src.sprite_frame_time;
    dst->field_6 = src.field_6;
    dst->screen_space_x = src.screen_space_x;
    dst->screen_space_y = src.screen_space_y;
    dst->field_C = src.field_C;
    dst->field_E = src.field_E;
    dst->field_10 = src.field_10;
}

void Deserialize(const OtherOverlay_MM7 &src, OtherOverlay *dst) {
    memzero(dst);

    dst->field_0 = src.field_0;
    dst->field_2 = src.field_2;
    dst->sprite_frame_time = src.sprite_frame_time;
    dst->field_6 = src.field_6;
    dst->screen_space_x = src.screen_space_x;
    dst->screen_space_y = src.screen_space_y;
    dst->field_C = src.field_C;
    dst->field_E = src.field_E;
    dst->field_10 = src.field_10;
}

void Serialize(const OtherOverlayList &src, OtherOverlayList_MM7 *dst) {
    memzero(dst);

    dst->bRedraw = true;
    dst->field_3E8 = src.field_3E8;
    Serialize(src.pOverlays, &dst->pOverlays);
}

void Deserialize(const OtherOverlayList_MM7 &src, OtherOverlayList *dst) {
    dst->field_3E8 = src.field_3E8;
    Deserialize(src.pOverlays, &dst->pOverlays);
}

void Serialize(const SpellBuff &src, SpellBuff_MM7 *dst) {
    memzero(dst);

    dst->expireTime = src.expireTime.value;
    dst->power = src.power;
    dst->skillMastery = std::to_underlying(src.skillMastery);
    dst->overlayId = src.overlayID;
    dst->caster = src.caster;
    dst->flags = src.isGMBuff;
}

void Deserialize(const SpellBuff_MM7 &src, SpellBuff *dst) {
    dst->expireTime.value = src.expireTime;
    dst->power = src.power;
    dst->skillMastery = static_cast<PLAYER_SKILL_MASTERY>(src.skillMastery);
    dst->overlayID = src.overlayId;
    dst->caster = src.caster;
    dst->isGMBuff = src.flags;
}

void Serialize(const ItemGen &src, ItemGen_MM7 *dst) {
    memzero(dst);

    dst->itemID = std::to_underlying(src.uItemID);
    dst->enchantmentType = src.uEnchantmentType;
    dst->enchantmentStrength = src.m_enchantmentStrength;
    dst->specialEnchantment = src.special_enchantment;
    dst->numCharges = src.uNumCharges;
    dst->attributes = std::to_underlying(src.uAttributes);
    dst->bodyAnchor = std::to_underlying(src.uBodyAnchor);
    dst->maxCharges = src.uMaxCharges;
    dst->holderPlayer = src.uHolderPlayer;
    dst->placedInChest = src.placedInChest;
    dst->expireTime = src.uExpireTime.value;
}

void Deserialize(const ItemGen_MM7 &src, ItemGen *dst) {
    dst->uItemID = static_cast<ITEM_TYPE>(src.itemID);
    dst->uEnchantmentType = src.enchantmentType;
    dst->m_enchantmentStrength = src.enchantmentStrength;
    dst->special_enchantment = static_cast<ITEM_ENCHANTMENT>(src.specialEnchantment);
    dst->uNumCharges = src.numCharges;
    dst->uAttributes = static_cast<ITEM_FLAGS>(src.attributes);
    dst->uBodyAnchor = static_cast<ITEM_SLOT>(src.bodyAnchor);
    dst->uMaxCharges = src.maxCharges;
    dst->uHolderPlayer = src.holderPlayer;
    dst->placedInChest = src.placedInChest;
    dst->uExpireTime.value = src.expireTime;
}

void Serialize(const Party &src, Party_MM7 *dst) {
    memzero(dst);

    dst->field_0 = src.field_0_set25_unused;
    dst->uPartyHeight = src.uPartyHeight;
    dst->uDefaultPartyHeight = src.uDefaultPartyHeight;
    dst->sEyelevel = src.sEyelevel;
    dst->uDefaultEyelevel = src.uDefaultEyelevel;
    dst->radius = src.radius;
    dst->_yawGranularity = src._yawGranularity;
    dst->uWalkSpeed = src.uWalkSpeed;
    dst->_yawRotationSpeed = src._yawRotationSpeed;
    dst->jump_strength = src.jump_strength;
    dst->field_28 = src.field_28_set0_unused;
    dst->uTimePlayed = src.playing_time.value;
    dst->uLastRegenerationTime = src.last_regenerated.value;

    // MM7 uses an array of size 10 here, but we only store 5 elements. So zero it first.
    dst->PartyTimes.bountyHunting_next_generation_time.fill(0);
    Serialize(src.PartyTimes.bountyHunting_next_generation_time, &dst->PartyTimes.bountyHunting_next_generation_time, 5);

    Serialize(src.PartyTimes.Shops_next_generation_time, &dst->PartyTimes.Shops_next_generation_time);
    Serialize(src.PartyTimes._shop_ban_times, &dst->PartyTimes._shop_ban_times);
    Serialize(src.PartyTimes.CounterEventValues, &dst->PartyTimes.CounterEventValues);
    Serialize(src.PartyTimes.HistoryEventTimes, &dst->PartyTimes.HistoryEventTimes);
    Serialize(src.PartyTimes._s_times, &dst->PartyTimes._s_times);

    dst->vPosition.x = src.vPosition.x;
    dst->vPosition.y = src.vPosition.y;
    dst->vPosition.z = src.vPosition.z;
    dst->_viewYaw = src._viewYaw;
    dst->_viewPitch = src._viewPitch;
    dst->vPrevPosition.x = src.vPrevPosition.x;
    dst->vPrevPosition.y = src.vPrevPosition.y;
    dst->vPrevPosition.z = src.vPrevPosition.z;
    dst->_viewPrevYaw = src._viewPrevYaw;
    dst->_viewPrevPitch = src._viewPrevPitch;
    dst->sPrevEyelevel = src.sPrevEyelevel;
    dst->field_6E0 = src.field_6E0_set0_unused;
    dst->field_6E4 = src.field_6E4_set0_unused;
    dst->uFallSpeed = src.uFallSpeed;
    dst->field_6EC = src.field_6EC_set0_unused;
    dst->field_6F0 = src.sPartySavedFlightZ;
    dst->floor_face_pid = src.floor_face_pid;
    dst->walk_sound_timer = 0; // zero walking sound timer, in OE it was removed and it is little meaning saving it
    dst->_6FC_water_lava_timer = src._6FC_water_lava_timer;
    dst->uFallStartZ = src.uFallStartZ;
    dst->bFlying = src.bFlying;
    dst->field_708 = src.field_708_set15_unused;
    dst->hirelingScrollPosition = src.hirelingScrollPosition;
    dst->field_70A = src.cNonHireFollowers;
    dst->field_70B = src.field_70B_set0_unused;
    dst->uCurrentYear = src.uCurrentYear;
    dst->uCurrentMonth = src.uCurrentMonth;
    dst->uCurrentMonthWeek = src.uCurrentMonthWeek;
    dst->uCurrentDayOfMonth = src.uCurrentDayOfMonth;
    dst->uCurrentHour = src.uCurrentHour;
    dst->uCurrentMinute = src.uCurrentMinute;
    dst->uCurrentTimeSecond = src.uCurrentTimeSecond;
    dst->uNumFoodRations = src.GetFood();
    dst->field_72C = src.field_72C_set0_unused;
    dst->field_730 = src.field_730_set0_unused;
    dst->uNumGold = src.GetGold();
    dst->uNumGoldInBank = src.uNumGoldInBank;
    dst->uNumDeaths = src.uNumDeaths;
    dst->field_740 = src.field_740_set0_unused;
    dst->uNumPrisonTerms = src.uNumPrisonTerms;
    dst->uNumBountiesCollected = src.uNumBountiesCollected;
    dst->field_74C = src.field_74C_set0_unused;

    Serialize(src.monster_id_for_hunting, &dst->monster_id_for_hunting);
    Serialize(src.monster_for_hunting_killed, &dst->monster_for_hunting_killed);

    dst->days_played_without_rest = src.days_played_without_rest;

    Serialize(src._quest_bits, &dst->_quest_bits);
    Serialize(src.pArcomageWins, &dst->pArcomageWins);

    dst->field_7B5_in_arena_quest = src.field_7B5_in_arena_quest;
    dst->uNumArenaWins = src.uNumArenaWins;

    Serialize(src.pIsArtifactFound, &dst->pIsArtifactFound);
    Serialize(src.field_7d7_set0_unused, &dst->field_7d7);
    Serialize(src._autonote_bits, &dst->_autonote_bits);
    Serialize(src.field_818_set0_unused, &dst->field_818);
    Serialize(src.random_order_num_unused, &dst->field_854);

    dst->uNumArcomageWins = src.uNumArcomageWins;
    dst->uNumArcomageLoses = src.uNumArcomageLoses;
    dst->bTurnBasedModeOn = src.bTurnBasedModeOn;
    dst->field_880 = src.field_880_set0_unused;
    dst->uFlags2 = src.uFlags2;

    uint align = 0;
    if (src.alignment == PartyAlignment::PartyAlignment_Evil) align = 2;
    if (src.alignment == PartyAlignment::PartyAlignment_Neutral) align = 1;
    dst->alignment = align;

    Serialize(src.pPartyBuffs, &dst->pPartyBuffs);
    Serialize(src.pPlayers, &dst->pPlayers);
    Serialize(src.pHirelings, &dst->pHirelings);

    Serialize(src.pPickedItem, &dst->pPickedItem);

    dst->uFlags = src.uFlags;

    Serialize(src.StandartItemsInShops, &dst->StandartItemsInShops);
    Serialize(src.SpecialItemsInShops, &dst->SpecialItemsInShops);
    Serialize(src.SpellBooksInGuilds, &dst->SpellBooksInGuilds);
    Serialize(src.field_1605C_set0_unused, &dst->field_1605C);

    Serialize(src.pHireling1Name, &dst->pHireling1Name);
    Serialize(src.pHireling2Name, &dst->pHireling2Name);

    dst->armageddon_timer = src.armageddon_timer;
    dst->armageddonDamage = src.armageddonDamage;

    Serialize(src.pTurnBasedPlayerRecoveryTimes, &dst->pTurnBasedPlayerRecoveryTimes);
    Serialize(src.InTheShopFlags, &dst->InTheShopFlags);

    dst->uFine = src.uFine;
    dst->flt_TorchlightColorR = src.flt_TorchlightColorR;
    dst->flt_TorchlightColorG = src.flt_TorchlightColorG;
    dst->flt_TorchlightColorB = src.flt_TorchlightColorB;
}

void Deserialize(const Party_MM7 &src, Party *dst) {
    dst->field_0_set25_unused = src.field_0;
    dst->uPartyHeight = src.uPartyHeight;
    dst->uDefaultPartyHeight = src.uDefaultPartyHeight;
    dst->sEyelevel = src.sEyelevel;
    dst->uDefaultEyelevel = src.uDefaultEyelevel;
    dst->radius = src.radius;
    dst->_yawGranularity = src._yawGranularity;
    dst->uWalkSpeed = src.uWalkSpeed;
    dst->_yawRotationSpeed = src._yawRotationSpeed;
    dst->jump_strength = src.jump_strength;
    dst->field_28_set0_unused = src.field_28;
    dst->playing_time.value = src.uTimePlayed;
    dst->last_regenerated.value = src.uLastRegenerationTime;

    Deserialize(src.PartyTimes.bountyHunting_next_generation_time, &dst->PartyTimes.bountyHunting_next_generation_time, 5);
    Deserialize(src.PartyTimes.Shops_next_generation_time, &dst->PartyTimes.Shops_next_generation_time);
    Deserialize(src.PartyTimes._shop_ban_times, &dst->PartyTimes._shop_ban_times);
    Deserialize(src.PartyTimes.CounterEventValues, &dst->PartyTimes.CounterEventValues);
    Deserialize(src.PartyTimes.HistoryEventTimes, &dst->PartyTimes.HistoryEventTimes);
    Deserialize(src.PartyTimes._s_times, &dst->PartyTimes._s_times);

    dst->vPosition.x = src.vPosition.x;
    dst->vPosition.y = src.vPosition.y;
    dst->vPosition.z = src.vPosition.z;
    dst->_viewYaw = src._viewYaw;
    dst->_viewPitch = src._viewPitch;
    dst->vPrevPosition.x = src.vPrevPosition.x;
    dst->vPrevPosition.y = src.vPrevPosition.y;
    dst->vPrevPosition.z = src.vPrevPosition.z;
    dst->_viewPrevYaw = src._viewPrevYaw;
    dst->_viewPrevPitch = src._viewPrevPitch;
    dst->sPrevEyelevel = src.sPrevEyelevel;
    dst->field_6E0_set0_unused = src.field_6E0;
    dst->field_6E4_set0_unused = src.field_6E4;
    dst->uFallSpeed = src.uFallSpeed;
    dst->field_6EC_set0_unused = src.field_6EC;
    dst->sPartySavedFlightZ = src.field_6F0;
    dst->floor_face_pid = src.floor_face_pid;
    // Walking sound timer was removed from OE
    //dst->walk_sound_timer = src.walk_sound_timer;
    dst->_6FC_water_lava_timer = src._6FC_water_lava_timer;
    dst->uFallStartZ = src.uFallStartZ;
    dst->bFlying = src.bFlying;
    dst->field_708_set15_unused = src.field_708;
    dst->hirelingScrollPosition = src.hirelingScrollPosition;
    dst->cNonHireFollowers = src.field_70A;
    dst->field_70B_set0_unused = src.field_70B;
    dst->uCurrentYear = src.uCurrentYear;
    dst->uCurrentMonth = src.uCurrentMonth;
    dst->uCurrentMonthWeek = src.uCurrentMonthWeek;
    dst->uCurrentDayOfMonth = src.uCurrentDayOfMonth;
    dst->uCurrentHour = src.uCurrentHour;
    dst->uCurrentMinute = src.uCurrentMinute;
    dst->uCurrentTimeSecond = src.uCurrentTimeSecond;
    dst->uNumFoodRations = src.uNumFoodRations;
    dst->field_72C_set0_unused = src.field_72C;
    dst->field_730_set0_unused = src.field_730;
    dst->uNumGold = src.uNumGold;
    dst->uNumGoldInBank = src.uNumGoldInBank;
    dst->uNumDeaths = src.uNumDeaths;
    dst->field_740_set0_unused = src.field_740;
    dst->uNumPrisonTerms = src.uNumPrisonTerms;
    dst->uNumBountiesCollected = src.uNumBountiesCollected;
    dst->field_74C_set0_unused = src.field_74C;

    Deserialize(src.monster_id_for_hunting, &dst->monster_id_for_hunting);
    Deserialize(src.monster_for_hunting_killed, &dst->monster_for_hunting_killed);

    dst->days_played_without_rest = src.days_played_without_rest;

    Deserialize(src._quest_bits, &dst->_quest_bits);
    Deserialize(src.pArcomageWins, &dst->pArcomageWins);

    dst->field_7B5_in_arena_quest = src.field_7B5_in_arena_quest;
    dst->uNumArenaWins = src.uNumArenaWins;

    Deserialize(src.pIsArtifactFound, &dst->pIsArtifactFound);
    Deserialize(src.field_7d7, &dst->field_7d7_set0_unused);
    Deserialize(src._autonote_bits, &dst->_autonote_bits);
    Deserialize(src.field_818, &dst->field_818_set0_unused);
    Deserialize(src.field_854, &dst->random_order_num_unused);

    dst->uNumArcomageWins = src.uNumArcomageWins;
    dst->uNumArcomageLoses = src.uNumArcomageLoses;
    dst->bTurnBasedModeOn = src.bTurnBasedModeOn;
    dst->field_880_set0_unused = src.field_880;
    dst->uFlags2 = src.uFlags2;

    switch (src.alignment) {
        case 0:
            dst->alignment = PartyAlignment::PartyAlignment_Good;
            break;
        case 1:
            dst->alignment = PartyAlignment::PartyAlignment_Neutral;
            break;
        case 2:
            dst->alignment = PartyAlignment::PartyAlignment_Evil;
            break;
        default:
            Assert(false);
    }

    Deserialize(src.pPartyBuffs, &dst->pPartyBuffs);
    Deserialize(src.pPlayers, &dst->pPlayers);
    Deserialize(src.pHirelings, &dst->pHirelings);

    Deserialize(src.pPickedItem, &dst->pPickedItem);

    dst->uFlags = src.uFlags;

    Deserialize(src.StandartItemsInShops, &dst->StandartItemsInShops);
    Deserialize(src.SpecialItemsInShops, &dst->SpecialItemsInShops);
    Deserialize(src.SpellBooksInGuilds, &dst->SpellBooksInGuilds);

    Deserialize(src.field_1605C, &dst->field_1605C_set0_unused);

    Deserialize(src.pHireling1Name, &dst->pHireling1Name);
    Deserialize(src.pHireling2Name, &dst->pHireling2Name);

    dst->armageddon_timer = src.armageddon_timer;
    dst->armageddonDamage = src.armageddonDamage;

    Deserialize(src.pTurnBasedPlayerRecoveryTimes, &dst->pTurnBasedPlayerRecoveryTimes);
    Deserialize(src.InTheShopFlags, &dst->InTheShopFlags);

    dst->uFine = src.uFine;

    // is this correct / ever used??
    dst->flt_TorchlightColorR = src.flt_TorchlightColorR;
    dst->flt_TorchlightColorG = src.flt_TorchlightColorG;
    dst->flt_TorchlightColorB = src.flt_TorchlightColorB;
}

void Serialize(const Player &src, Player_MM7 *dst) {
    memzero(dst);

    for (unsigned int i = 0; i < 20; ++i)
        dst->conditions[i] = src.conditions.Get(static_cast<Condition>(i)).value;

    dst->experience = src.experience;

    Serialize(src.name, &dst->name);

    dst->sex = src.uSex;
    dst->classType = src.classType;
    dst->currentFace = src.uCurrentFace;
    dst->might = src.uMight;
    dst->mightBonus = src.uMightBonus;
    dst->intelligence = src.uIntelligence;
    dst->intelligenceBonus = src.uIntelligenceBonus;
    dst->willpower = src.uWillpower;
    dst->willpowerBonus = src.uWillpowerBonus;
    dst->endurance = src.uEndurance;
    dst->enduranceBonus = src.uEnduranceBonus;
    dst->speed = src.uSpeed;
    dst->speedBonus = src.uSpeedBonus;
    dst->accuracy = src.uAccuracy;
    dst->accuracyBonus = src.uAccuracyBonus;
    dst->luck = src.uLuck;
    dst->luckBonus = src.uLuckBonus;
    dst->acModifier = src.sACModifier;
    dst->level = src.uLevel;
    dst->levelModifier = src.sLevelModifier;
    dst->ageModifier = src.sAgeModifier;
    dst->field_E0 = src.field_E0;
    dst->field_E4 = src.field_E4;
    dst->field_E8 = src.field_E8;
    dst->field_EC = src.field_EC;
    dst->field_F0 = src.field_F0;
    dst->field_F4 = src.field_F4;
    dst->field_F8 = src.field_F8;
    dst->field_FC = src.field_FC;
    dst->field_100 = src.field_100;
    dst->field_104 = src.field_104;

    Serialize(src.pActiveSkills, &dst->activeSkills, 37);
    Serialize(src._achieved_awards_bits, &dst->achievedAwardsBits);
    Serialize(src.spellbook.bHaveSpell, &dst->spellbook.haveSpell);

    dst->pureLuckUsed = src.pure_luck_used;
    dst->pureSpeedUsed = src.pure_speed_used;
    dst->pureIntellectUsed = src.pure_intellect_used;
    dst->pureEnduranceUsed = src.pure_endurance_used;
    dst->pureWillpowerUsed = src.pure_willpower_used;
    dst->pureAccuracyUsed = src.pure_accuracy_used;
    dst->pureMightUsed = src.pure_might_used;

    Serialize(src.pOwnItems, &dst->ownItems);
    Serialize(src.pInventoryMatrix, &dst->inventoryMatrix);

    dst->resFireBase = src.sResFireBase;
    dst->resAirBase = src.sResAirBase;
    dst->resWaterBase = src.sResWaterBase;
    dst->resEarthBase = src.sResEarthBase;
    dst->resPhysicalBase = src.sResPhysicalBase;
    dst->resMagicBase = src.sResMagicBase;
    dst->resSpiritBase = src.sResSpiritBase;
    dst->resMindBase = src.sResMindBase;
    dst->resBodyBase = src.sResBodyBase;
    dst->resLightBase = src.sResLightBase;
    dst->resDarkBase = src.sResDarkBase;
    dst->resFireBonus = src.sResFireBonus;
    dst->resAirBonus = src.sResAirBonus;
    dst->resWaterBonus = src.sResWaterBonus;
    dst->resEarthBonus = src.sResEarthBonus;
    dst->resPhysicalBonus = src.sResPhysicalBonus;
    dst->resMagicBonus = src.sResMagicBonus;
    dst->resSpiritBonus = src.sResSpiritBonus;
    dst->resMindBonus = src.sResMindBonus;
    dst->resBodyBonus = src.sResBodyBonus;
    dst->resLightBonus = src.sResLightBonus;
    dst->resDarkBonus = src.sResDarkBonus;

    Serialize(src.pPlayerBuffs, &dst->playerBuffs);

    dst->voiceId = src.uVoiceID;
    dst->prevVoiceId = src.uPrevVoiceID;
    dst->prevFace = src.uPrevFace;
    dst->field_192C = src.field_192C;
    dst->field_1930 = src.field_1930;
    dst->timeToRecovery = src.timeToRecovery;
    dst->field_1936 = src.field_1936;
    dst->field_1937 = src.field_1937;
    dst->skillPoints = src.uSkillPoints;
    dst->health = src.health;
    dst->mana = src.mana;
    dst->birthYear = src.uBirthYear;

    Serialize(src.pEquipment.pIndices, &dst->equipment.indices);

    Serialize(src.field_1988, &dst->field_1988);

    dst->field_1A4C = src.field_1A4C;
    dst->field_1A4D = src.field_1A4D;
    dst->lastOpenedSpellbookPage = src.lastOpenedSpellbookPage;
    dst->quickSpell = std::to_underlying(src.uQuickSpell);

    Serialize(src.playerEventBits, &dst->playerEventBits);

    dst->someAttackBonus = src._some_attack_bonus;
    dst->field_1A91 = src.field_1A91;
    dst->meleeDmgBonus = src._melee_dmg_bonus;
    dst->field_1A93 = src.field_1A93;
    dst->rangedAttackBonus = src._ranged_atk_bonus;
    dst->field_1A95 = src.field_1A95;
    dst->rangedDmgBonus = src._ranged_dmg_bonus;
    dst->field_1A97 = src.field_1A97_set0_unused;
    dst->fullHealthBonus = src.uFullHealthBonus;
    dst->healthRelated = src._health_related;
    dst->fullManaBonus = src.uFullManaBonus;
    dst->manaRelated = src._mana_related;
    dst->expression = src.expression;
    dst->expressionTimePassed = src.uExpressionTimePassed;
    dst->expressionTimeLength = src.uExpressionTimeLength;
    dst->field_1AA2 = src.uExpressionImageIndex;
    dst->_expression21_animtime = src._expression21_animtime;
    dst->_expression21_frameset = src._expression21_frameset;

    for (unsigned int i = 0; i < 5; ++i) {
        if (i >= src.vBeacons.size()) {
            continue;
        }
        dst->installedBeacons[i].beaconTime = src.vBeacons[i].uBeaconTime.value;
        dst->installedBeacons[i].partyPosX = src.vBeacons[i].PartyPos_X;
        dst->installedBeacons[i].partyPosY = src.vBeacons[i].PartyPos_Y;
        dst->installedBeacons[i].partyPosZ = src.vBeacons[i].PartyPos_Z;
        dst->installedBeacons[i].partyViewYaw = src.vBeacons[i]._partyViewYaw;
        dst->installedBeacons[i].partyViewPitch = src.vBeacons[i]._partyViewPitch;
        dst->installedBeacons[i].saveFileId = src.vBeacons[i].SaveFileID;
    }

    dst->numDivineInterventionCasts = src.uNumDivineInterventionCastsThisDay;
    dst->numArmageddonCasts = src.uNumArmageddonCasts;
    dst->numFireSpikeCasts = src.uNumFireSpikeCasts;
    dst->field_1B3B = src.field_1B3B_set0_unused;
}

void Deserialize(const Player_MM7 &src, Player *dst) {
    for (unsigned int i = 0; i < 20; ++i)
        dst->conditions.Set(static_cast<Condition>(i), GameTime(src.conditions[i]));

    dst->experience = src.experience;

    Deserialize(src.name, &dst->name);

    switch (src.sex) {
    case 0:
        dst->uSex = SEX_MALE;
        break;
    case 1:
        dst->uSex = SEX_FEMALE;
        break;
    default:
        Assert(false);
    }

    switch (src.classType) {
    case 0:
        dst->classType = PLAYER_CLASS_KNIGHT;
        break;
    case 1:
        dst->classType = PLAYER_CLASS_CHEVALIER;
        break;
    case 2:
        dst->classType = PLAYER_CLASS_CHAMPION;
        break;
    case 3:
        dst->classType = PLAYER_CLASS_BLACK_KNIGHT;
        break;
    case 4:
        dst->classType = PLAYER_CLASS_THIEF;
        break;
    case 5:
        dst->classType = PLAYER_CLASS_ROGUE;
        break;
    case 6:
        dst->classType = PLAYER_CLASS_SPY;
        break;
    case 7:
        dst->classType = PLAYER_CLASS_ASSASSIN;
        break;
    case 8:
        dst->classType = PLAYER_CLASS_MONK;
        break;
    case 9:
        dst->classType = PLAYER_CLASS_INITIATE;
        break;
    case 10:
        dst->classType = PLAYER_CLASS_MASTER;
        break;
    case 11:
        dst->classType = PLAYER_CLASS_NINJA;
        break;
    case 12:
        dst->classType = PLAYER_CLASS_PALADIN;
        break;
    case 13:
        dst->classType = PLAYER_CLASS_CRUSADER;
        break;
    case 14:
        dst->classType = PLAYER_CLASS_HERO;
        break;
    case 15:
        dst->classType = PLAYER_CLASS_VILLIAN;
        break;
    case 16:
        dst->classType = PLAYER_CLASS_ARCHER;
        break;
    case 17:
        dst->classType = PLAYER_CLASS_WARRIOR_MAGE;
        break;
    case 18:
        dst->classType = PLAYER_CLASS_MASTER_ARCHER;
        break;
    case 19:
        dst->classType = PLAYER_CLASS_SNIPER;
        break;
    case 20:
        dst->classType = PLAYER_CLASS_RANGER;
        break;
    case 21:
        dst->classType = PLAYER_CLASS_HUNTER;
        break;
    case 22:
        dst->classType = PLAYER_CLASS_RANGER_LORD;
        break;
    case 23:
        dst->classType = PLAYER_CLASS_BOUNTY_HUNTER;
        break;
    case 24:
        dst->classType = PLAYER_CLASS_CLERIC;
        break;
    case 25:
        dst->classType = PLAYER_CLASS_PRIEST;
        break;
    case 26:
        dst->classType = PLAYER_CLASS_PRIEST_OF_SUN;
        break;
    case 27:
        dst->classType = PLAYER_CLASS_PRIEST_OF_MOON;
        break;
    case 28:
        dst->classType = PLAYER_CLASS_DRUID;
        break;
    case 29:
        dst->classType = PLAYER_CLASS_GREAT_DRUID;
        break;
    case 30:
        dst->classType = PLAYER_CLASS_ARCH_DRUID;
        break;
    case 31:
        dst->classType = PLAYER_CLASS_WARLOCK;
        break;
    case 32:
        dst->classType = PLAYER_CLASS_SORCERER;
        break;
    case 33:
        dst->classType = PLAYER_CLASS_WIZARD;
        break;
    case 34:
        dst->classType = PLAYER_CLASS_ARCHMAGE;
        break;
    case 35:
        dst->classType = PLAYER_CLASS_LICH;
        break;
    default:
        Assert(false);
    }

    dst->uCurrentFace = src.currentFace;
    dst->uMight = src.might;
    dst->uMightBonus = src.mightBonus;
    dst->uIntelligence = src.intelligence;
    dst->uIntelligenceBonus = src.intelligenceBonus;
    dst->uWillpower = src.willpower;
    dst->uWillpowerBonus = src.willpowerBonus;
    dst->uEndurance = src.endurance;
    dst->uEnduranceBonus = src.enduranceBonus;
    dst->uSpeed = src.speed;
    dst->uSpeedBonus = src.speedBonus;
    dst->uAccuracy = src.accuracy;
    dst->uAccuracyBonus = src.accuracyBonus;
    dst->uLuck = src.luck;
    dst->uLuckBonus = src.luckBonus;
    dst->sACModifier = src.acModifier;
    dst->uLevel = src.level;
    dst->sLevelModifier = src.levelModifier;
    dst->sAgeModifier = src.ageModifier;
    dst->field_E0 = src.field_E0;
    dst->field_E4 = src.field_E4;
    dst->field_E8 = src.field_E8;
    dst->field_EC = src.field_EC;
    dst->field_F0 = src.field_F0;
    dst->field_F4 = src.field_F4;
    dst->field_F8 = src.field_F8;
    dst->field_FC = src.field_FC;
    dst->field_100 = src.field_100;
    dst->field_104 = src.field_104;

    Deserialize(src.activeSkills, &dst->pActiveSkills, 37);
    Deserialize(src.achievedAwardsBits, &dst->_achieved_awards_bits);
    Deserialize(src.spellbook.haveSpell, &dst->spellbook.bHaveSpell);

    dst->pure_luck_used = src.pureLuckUsed;
    dst->pure_speed_used = src.pureSpeedUsed;
    dst->pure_intellect_used = src.pureIntellectUsed;
    dst->pure_endurance_used = src.pureEnduranceUsed;
    dst->pure_willpower_used = src.pureWillpowerUsed;
    dst->pure_accuracy_used = src.pureAccuracyUsed;
    dst->pure_might_used = src.pureMightUsed;

    Deserialize(src.ownItems, &dst->pOwnItems);
    Deserialize(src.inventoryMatrix, &dst->pInventoryMatrix);

    dst->sResFireBase = src.resFireBase;
    dst->sResAirBase = src.resAirBase;
    dst->sResWaterBase = src.resWaterBase;
    dst->sResEarthBase = src.resEarthBase;
    dst->sResPhysicalBase = src.resPhysicalBase;
    dst->sResMagicBase = src.resMagicBase;
    dst->sResSpiritBase = src.resSpiritBase;
    dst->sResMindBase = src.resMindBase;
    dst->sResBodyBase = src.resBodyBase;
    dst->sResLightBase = src.resLightBase;
    dst->sResDarkBase = src.resDarkBase;
    dst->sResFireBonus = src.resFireBonus;
    dst->sResAirBonus = src.resAirBonus;
    dst->sResWaterBonus = src.resWaterBonus;
    dst->sResEarthBonus = src.resEarthBonus;
    dst->sResPhysicalBonus = src.resPhysicalBonus;
    dst->sResMagicBonus = src.resMagicBonus;
    dst->sResSpiritBonus = src.resSpiritBonus;
    dst->sResMindBonus = src.resMindBonus;
    dst->sResBodyBonus = src.resBodyBonus;
    dst->sResLightBonus = src.resLightBonus;
    dst->sResDarkBonus = src.resDarkBonus;

    Deserialize(src.playerBuffs, &dst->pPlayerBuffs);

    dst->uVoiceID = src.voiceId;
    dst->uPrevVoiceID = src.prevVoiceId;
    dst->uPrevFace = src.prevFace;
    dst->field_192C = src.field_192C;
    dst->field_1930 = src.field_1930;
    dst->timeToRecovery = src.timeToRecovery;
    dst->field_1936 = src.field_1936;
    dst->field_1937 = src.field_1937;
    dst->uSkillPoints = src.skillPoints;
    dst->health = src.health;
    dst->mana = src.mana;
    dst->uBirthYear = src.birthYear;

    Deserialize(src.equipment.indices, &dst->pEquipment.pIndices);

    Deserialize(src.field_1988, &dst->field_1988);

    dst->field_1A4C = src.field_1A4C;
    dst->field_1A4D = src.field_1A4D;
    dst->lastOpenedSpellbookPage = src.lastOpenedSpellbookPage;
    dst->uQuickSpell = static_cast<SPELL_TYPE>(src.quickSpell);

    Deserialize(src.playerEventBits, &dst->playerEventBits);

    dst->_some_attack_bonus = src.someAttackBonus;
    dst->field_1A91 = src.field_1A91;
    dst->_melee_dmg_bonus = src.meleeDmgBonus;
    dst->field_1A93 = src.field_1A93;
    dst->_ranged_atk_bonus = src.rangedAttackBonus;
    dst->field_1A95 = src.field_1A95;
    dst->_ranged_dmg_bonus = src.rangedDmgBonus;
    dst->field_1A97_set0_unused = src.field_1A97;
    dst->uFullHealthBonus = src.fullHealthBonus;
    dst->_health_related = src.healthRelated;
    dst->uFullManaBonus = src.fullManaBonus;
    dst->_mana_related = src.manaRelated;
    dst->expression = (CHARACTER_EXPRESSION_ID)src.expression;
    dst->uExpressionTimePassed = src.expressionTimePassed;
    dst->uExpressionTimeLength = src.expressionTimeLength;
    dst->uExpressionImageIndex = src.field_1AA2;
    dst->_expression21_animtime = src._expression21_animtime;
    dst->_expression21_frameset = src._expression21_frameset;

    for (int z = 0; z < dst->vBeacons.size(); z++)
        dst->vBeacons[z].image->Release();
    dst->vBeacons.clear();

    for (unsigned int i = 0; i < 5; ++i) {
        if (src.installedBeacons[i].beaconTime != 0) {
            LloydBeacon beacon;
            beacon.uBeaconTime = GameTime(src.installedBeacons[i].beaconTime);
            beacon.PartyPos_X = src.installedBeacons[i].partyPosX;
            beacon.PartyPos_Y = src.installedBeacons[i].partyPosY;
            beacon.PartyPos_Z = src.installedBeacons[i].partyPosZ;
            beacon._partyViewYaw = src.installedBeacons[i].partyViewYaw;
            beacon._partyViewPitch = src.installedBeacons[i].partyViewPitch;
            beacon.SaveFileID = src.installedBeacons[i].saveFileId;
            dst->vBeacons.push_back(beacon);
        }
    }

    dst->uNumDivineInterventionCastsThisDay = src.numDivineInterventionCasts;
    dst->uNumArmageddonCasts = src.numArmageddonCasts;
    dst->uNumFireSpikeCasts = src.numFireSpikeCasts;
    dst->field_1B3B_set0_unused = src.field_1B3B;
}

void Serialize(const Icon &src, IconFrame_MM7 *dst) {
    memzero(dst);

    strcpy(dst->pAnimationName.data(), src.GetAnimationName()); // TODO(captainurist): as unsafe as it gets
    dst->uAnimLength = src.GetAnimLength();

    strcpy(dst->pTextureName.data(), src.pTextureName);
    dst->uAnimTime = src.GetAnimTime();
    dst->uFlags = src.uFlags;
}

void Deserialize(const IconFrame_MM7 &src, Icon *dst) {
    dst->SetAnimationName(src.pAnimationName.data());
    dst->SetAnimLength(8 * src.uAnimLength);

    strcpy(dst->pTextureName, src.pTextureName.data());
    dst->SetAnimTime(src.uAnimTime);
    dst->uFlags = src.uFlags;
}

void Serialize(const UIAnimation &src, UIAnimation_MM7 *dst) {
    memzero(dst);

    /* 000 */ dst->uIconID = src.icon->id;
    /* 002 */ dst->field_2 = src.field_2;
    /* 004 */ dst->uAnimTime = src.uAnimTime;
    /* 006 */ dst->uAnimLength = src.uAnimLength;
    /* 008 */ dst->x = src.x;
    /* 00A */ dst->y = src.y;
    /* 00C */ dst->field_C = src.field_C;
}

void Deserialize(const UIAnimation_MM7 &src, UIAnimation *dst) {
    dst->icon = pIconsFrameTable->GetIcon(src.uIconID);
    ///* 000 */ anim->uIconID = src.uIconID;
    /* 002 */ dst->field_2 = src.field_2;
    /* 004 */ dst->uAnimTime = src.uAnimTime;
    /* 006 */ dst->uAnimLength = src.uAnimLength;
    /* 008 */ dst->x = src.x;
    /* 00A */ dst->y = src.y;
    /* 00C */ dst->field_C = src.field_C;
}

void Deserialize(const MonsterDesc_MM6 &src, MonsterDesc *dst) {
    dst->uMonsterHeight = src.uMonsterHeight;
    dst->uMonsterRadius = src.uMonsterRadius;
    dst->uMovementSpeed = src.uMovementSpeed;
    dst->uToHitRadius = src.uToHitRadius;
    dst->sTintColor = colorTable.White.c32();
    dst->pSoundSampleIDs = src.pSoundSampleIDs;
    Deserialize(src.pMonsterName, &dst->pMonsterName);
    Deserialize(src.pSpriteNames, &dst->pSpriteNames, 8);
}

void Serialize(const MonsterDesc &src, MonsterDesc_MM7 *dst) {
    memzero(dst);

    dst->uMonsterHeight = src.uMonsterHeight;
    dst->uMonsterRadius = src.uMonsterRadius;
    dst->uMovementSpeed = src.uMovementSpeed;
    dst->uToHitRadius = src.uToHitRadius;
    dst->sTintColor = src.sTintColor;
    dst->pSoundSampleIDs = src.pSoundSampleIDs;
    Serialize(src.pMonsterName, &dst->pMonsterName);
    Serialize(src.pSpriteNames, &dst->pSpriteNames, 8);
    dst->pSpriteNames[8][0] = '\0';
    dst->pSpriteNames[9][0] = '\0';
}

void Deserialize(const MonsterDesc_MM7 &src, MonsterDesc *dst) {
    dst->uMonsterHeight = src.uMonsterHeight;
    dst->uMonsterRadius = src.uMonsterRadius;
    dst->uMovementSpeed = src.uMovementSpeed;
    dst->uToHitRadius = src.uToHitRadius;
    dst->sTintColor = src.sTintColor;
    dst->pSoundSampleIDs = src.pSoundSampleIDs;
    Deserialize(src.pMonsterName, &dst->pMonsterName);
    Deserialize(src.pSpriteNames, &dst->pSpriteNames, 8);
}

void Serialize(const ActorJob &src, ActorJob_MM7 *dst) {
    memzero(dst);
    dst->vPos = src.vPos;
    dst->uAttributes = src.uAttributes;
    dst->uAction = src.uAction;
    dst->uHour = src.uHour;
    dst->uDay = src.uDay;
    dst->uMonth = src.uMonth;
}

void Deserialize(const ActorJob_MM7 &src, ActorJob *dst) {
    dst->vPos = src.vPos;
    dst->uAttributes = src.uAttributes;
    dst->uAction = src.uAction;
    dst->uHour = src.uHour;
    dst->uDay = src.uDay;
    dst->uMonth = src.uMonth;
}

void Serialize(const Actor &src, Actor_MM7 *dst) {
    memzero(dst);

    Serialize(src.pActorName, &dst->pActorName);

    dst->sNPC_ID = src.sNPC_ID;
    dst->field_22 = src.field_22;
    dst->uAttributes = std::to_underlying(src.uAttributes);
    dst->sCurrentHP = src.sCurrentHP;

    dst->pMonsterInfo.uLevel = src.pMonsterInfo.uLevel;
    dst->pMonsterInfo.uTreasureDropChance = src.pMonsterInfo.uTreasureDropChance;
    dst->pMonsterInfo.uTreasureDiceRolls = src.pMonsterInfo.uTreasureDiceRolls;
    dst->pMonsterInfo.uTreasureDiceSides = src.pMonsterInfo.uTreasureDiceSides;
    dst->pMonsterInfo.uTreasureLevel = std::to_underlying(src.pMonsterInfo.uTreasureLevel);
    dst->pMonsterInfo.uTreasureType = src.pMonsterInfo.uTreasureType;
    dst->pMonsterInfo.uFlying = src.pMonsterInfo.uFlying;
    dst->pMonsterInfo.uMovementType = src.pMonsterInfo.uMovementType;
    dst->pMonsterInfo.uAIType = src.pMonsterInfo.uAIType;
    dst->pMonsterInfo.uHostilityType = (uint8_t)src.pMonsterInfo.uHostilityType;
    dst->pMonsterInfo.field_12 = src.pMonsterInfo.field_12;
    dst->pMonsterInfo.uSpecialAttackType = src.pMonsterInfo.uSpecialAttackType;
    dst->pMonsterInfo.uSpecialAttackLevel = src.pMonsterInfo.uSpecialAttackLevel;
    dst->pMonsterInfo.uAttack1Type = src.pMonsterInfo.uAttack1Type;
    dst->pMonsterInfo.uAttack1DamageDiceRolls = src.pMonsterInfo.uAttack1DamageDiceRolls;
    dst->pMonsterInfo.uAttack1DamageDiceSides = src.pMonsterInfo.uAttack1DamageDiceSides;
    dst->pMonsterInfo.uAttack1DamageBonus = src.pMonsterInfo.uAttack1DamageBonus;
    dst->pMonsterInfo.uMissleAttack1Type = src.pMonsterInfo.uMissleAttack1Type;
    dst->pMonsterInfo.uAttack2Chance = src.pMonsterInfo.uAttack2Chance;
    dst->pMonsterInfo.uAttack2Type = src.pMonsterInfo.uAttack2Type;
    dst->pMonsterInfo.uAttack2DamageDiceRolls = src.pMonsterInfo.uAttack2DamageDiceRolls;
    dst->pMonsterInfo.uAttack2DamageDiceSides = src.pMonsterInfo.uAttack2DamageDiceSides;
    dst->pMonsterInfo.uAttack2DamageBonus = src.pMonsterInfo.uAttack2DamageBonus;
    dst->pMonsterInfo.uMissleAttack2Type = src.pMonsterInfo.uMissleAttack2Type;
    dst->pMonsterInfo.uSpell1UseChance = src.pMonsterInfo.uSpell1UseChance;
    dst->pMonsterInfo.uSpell1ID = std::to_underlying(src.pMonsterInfo.uSpell1ID);
    dst->pMonsterInfo.uSpell2UseChance = src.pMonsterInfo.uSpell2UseChance;
    dst->pMonsterInfo.uSpell2ID = std::to_underlying(src.pMonsterInfo.uSpell2ID);
    dst->pMonsterInfo.uResFire = src.pMonsterInfo.uResFire;
    dst->pMonsterInfo.uResAir = src.pMonsterInfo.uResAir;
    dst->pMonsterInfo.uResWater = src.pMonsterInfo.uResWater;
    dst->pMonsterInfo.uResEarth = src.pMonsterInfo.uResEarth;
    dst->pMonsterInfo.uResMind = src.pMonsterInfo.uResMind;
    dst->pMonsterInfo.uResSpirit = src.pMonsterInfo.uResSpirit;
    dst->pMonsterInfo.uResBody = src.pMonsterInfo.uResBody;
    dst->pMonsterInfo.uResLight = src.pMonsterInfo.uResLight;
    dst->pMonsterInfo.uResDark = src.pMonsterInfo.uResDark;
    dst->pMonsterInfo.uResPhysical = src.pMonsterInfo.uResPhysical;
    dst->pMonsterInfo.uSpecialAbilityType = src.pMonsterInfo.uSpecialAbilityType;
    dst->pMonsterInfo.uSpecialAbilityDamageDiceRolls = src.pMonsterInfo.uSpecialAbilityDamageDiceRolls;
    dst->pMonsterInfo.uSpecialAbilityDamageDiceSides = src.pMonsterInfo.uSpecialAbilityDamageDiceSides;
    dst->pMonsterInfo.uSpecialAbilityDamageDiceBonus = src.pMonsterInfo.uSpecialAbilityDamageDiceBonus;
    dst->pMonsterInfo.uNumCharactersAttackedPerSpecialAbility = src.pMonsterInfo.uNumCharactersAttackedPerSpecialAbility;
    dst->pMonsterInfo.field_33 = src.pMonsterInfo.field_33;
    dst->pMonsterInfo.uID = src.pMonsterInfo.uID;
    dst->pMonsterInfo.bBloodSplatOnDeath = src.pMonsterInfo.bBloodSplatOnDeath;
    dst->pMonsterInfo.uSpellSkillAndMastery1 = src.pMonsterInfo.uSpellSkillAndMastery1;
    dst->pMonsterInfo.uSpellSkillAndMastery2 = src.pMonsterInfo.uSpellSkillAndMastery2;
    dst->pMonsterInfo.field_3C_some_special_attack = src.pMonsterInfo.field_3C_some_special_attack;
    dst->pMonsterInfo.field_3E = src.pMonsterInfo.field_3E;
    dst->pMonsterInfo.uHP = src.pMonsterInfo.uHP;
    dst->pMonsterInfo.uAC = src.pMonsterInfo.uAC;
    dst->pMonsterInfo.uExp = src.pMonsterInfo.uExp;
    dst->pMonsterInfo.uBaseSpeed = src.pMonsterInfo.uBaseSpeed;
    dst->pMonsterInfo.uRecoveryTime = src.pMonsterInfo.uRecoveryTime;
    dst->pMonsterInfo.uAttackPreference = src.pMonsterInfo.uAttackPreference;
    dst->word_000084_range_attack = src.word_000084_range_attack;
    dst->word_000086_some_monster_id = src.word_000086_some_monster_id;  // base monster class monsterlist id
    dst->uActorRadius = src.uActorRadius;
    dst->uActorHeight = src.uActorHeight;
    dst->uMovementSpeed = src.uMovementSpeed;
    dst->vPosition = src.vPosition;
    dst->vVelocity = src.vVelocity;
    dst->uYawAngle = src.uYawAngle;
    dst->uPitchAngle = src.uPitchAngle;
    dst->uSectorID = src.uSectorID;
    dst->uCurrentActionLength = src.uCurrentActionLength;
    dst->vInitialPosition = src.vInitialPosition;
    dst->vGuardingPosition = src.vGuardingPosition;
    dst->uTetherDistance = src.uTetherDistance;
    dst->uAIState = std::to_underlying(src.uAIState);
    dst->uCurrentActionAnimation = std::to_underlying(src.uCurrentActionAnimation);
    dst->uCarriedItemID = std::to_underlying(src.uCarriedItemID);
    dst->field_B6 = src.field_B6;
    dst->field_B7 = src.field_B7;
    dst->uCurrentActionTime = src.uCurrentActionTime;

    Serialize(src.pSpriteIDs, &dst->pSpriteIDs);
    Serialize(src.pSoundSampleIDs, &dst->pSoundSampleIDs);
    Serialize(src.pActorBuffs, &dst->pActorBuffs);
    Serialize(src.ActorHasItems, &dst->ActorHasItems);

    dst->uGroup = src.uGroup;
    dst->uAlly = src.uAlly;

    Serialize(src.pScheduledJobs, &dst->pScheduledJobs);

    dst->uSummonerID = src.uSummonerID;
    dst->uLastCharacterIDToHit = src.uLastCharacterIDToHit;
    dst->dword_000334_unique_name = src.dword_000334_unique_name;

    Serialize(src.field_338, &dst->field_338);
}

void Deserialize(const Actor_MM7 &src, Actor *dst) {
    Deserialize(src.pActorName, &dst->pActorName);
    dst->sNPC_ID = src.sNPC_ID;
    dst->field_22 = src.field_22;
    dst->uAttributes = ActorAttributes(src.uAttributes);
    dst->sCurrentHP = src.sCurrentHP;

    dst->pMonsterInfo.uLevel = src.pMonsterInfo.uLevel;
    dst->pMonsterInfo.uTreasureDropChance = src.pMonsterInfo.uTreasureDropChance;
    dst->pMonsterInfo.uTreasureDiceRolls = src.pMonsterInfo.uTreasureDiceRolls;
    dst->pMonsterInfo.uTreasureDiceSides = src.pMonsterInfo.uTreasureDiceSides;
    dst->pMonsterInfo.uTreasureLevel = static_cast<ITEM_TREASURE_LEVEL>(src.pMonsterInfo.uTreasureLevel);
    dst->pMonsterInfo.uTreasureType = src.pMonsterInfo.uTreasureType;
    dst->pMonsterInfo.uFlying = src.pMonsterInfo.uFlying;
    dst->pMonsterInfo.uMovementType = src.pMonsterInfo.uMovementType;
    dst->pMonsterInfo.uAIType = src.pMonsterInfo.uAIType;
    dst->pMonsterInfo.uHostilityType = static_cast<MonsterInfo::HostilityRadius>(src.pMonsterInfo.uHostilityType);
    dst->pMonsterInfo.field_12 = src.pMonsterInfo.field_12;
    dst->pMonsterInfo.uSpecialAttackType = static_cast<SPECIAL_ATTACK_TYPE>(src.pMonsterInfo.uSpecialAttackType);
    dst->pMonsterInfo.uSpecialAttackLevel = src.pMonsterInfo.uSpecialAttackLevel;
    dst->pMonsterInfo.uAttack1Type = src.pMonsterInfo.uAttack1Type;
    dst->pMonsterInfo.uAttack1DamageDiceRolls = src.pMonsterInfo.uAttack1DamageDiceRolls;
    dst->pMonsterInfo.uAttack1DamageDiceSides = src.pMonsterInfo.uAttack1DamageDiceSides;
    dst->pMonsterInfo.uAttack1DamageBonus = src.pMonsterInfo.uAttack1DamageBonus;
    dst->pMonsterInfo.uMissleAttack1Type = src.pMonsterInfo.uMissleAttack1Type;
    dst->pMonsterInfo.uAttack2Chance = src.pMonsterInfo.uAttack2Chance;
    dst->pMonsterInfo.uAttack2Type = src.pMonsterInfo.uAttack2Type;
    dst->pMonsterInfo.uAttack2DamageDiceRolls = src.pMonsterInfo.uAttack2DamageDiceRolls;
    dst->pMonsterInfo.uAttack2DamageDiceSides = src.pMonsterInfo.uAttack2DamageDiceSides;
    dst->pMonsterInfo.uAttack2DamageBonus = src.pMonsterInfo.uAttack2DamageBonus;
    dst->pMonsterInfo.uMissleAttack2Type = src.pMonsterInfo.uMissleAttack2Type;
    dst->pMonsterInfo.uSpell1UseChance = src.pMonsterInfo.uSpell1UseChance;
    dst->pMonsterInfo.uSpell1ID = static_cast<SPELL_TYPE>(src.pMonsterInfo.uSpell1ID);
    dst->pMonsterInfo.uSpell2UseChance = src.pMonsterInfo.uSpell2UseChance;
    dst->pMonsterInfo.uSpell2ID = static_cast<SPELL_TYPE>(src.pMonsterInfo.uSpell2ID);
    dst->pMonsterInfo.uResFire = src.pMonsterInfo.uResFire;
    dst->pMonsterInfo.uResAir = src.pMonsterInfo.uResAir;
    dst->pMonsterInfo.uResWater = src.pMonsterInfo.uResWater;
    dst->pMonsterInfo.uResEarth = src.pMonsterInfo.uResEarth;
    dst->pMonsterInfo.uResMind = src.pMonsterInfo.uResMind;
    dst->pMonsterInfo.uResSpirit = src.pMonsterInfo.uResSpirit;
    dst->pMonsterInfo.uResBody = src.pMonsterInfo.uResBody;
    dst->pMonsterInfo.uResLight = src.pMonsterInfo.uResLight;
    dst->pMonsterInfo.uResDark = src.pMonsterInfo.uResDark;
    dst->pMonsterInfo.uResPhysical = src.pMonsterInfo.uResPhysical;
    dst->pMonsterInfo.uSpecialAbilityType = src.pMonsterInfo.uSpecialAbilityType;
    dst->pMonsterInfo.uSpecialAbilityDamageDiceRolls = src.pMonsterInfo.uSpecialAbilityDamageDiceRolls;
    dst->pMonsterInfo.uSpecialAbilityDamageDiceSides = src.pMonsterInfo.uSpecialAbilityDamageDiceSides;
    dst->pMonsterInfo.uSpecialAbilityDamageDiceBonus = src.pMonsterInfo.uSpecialAbilityDamageDiceBonus;
    dst->pMonsterInfo.uNumCharactersAttackedPerSpecialAbility = src.pMonsterInfo.uNumCharactersAttackedPerSpecialAbility;
    dst->pMonsterInfo.field_33 = src.pMonsterInfo.field_33;
    dst->pMonsterInfo.uID = src.pMonsterInfo.uID;
    dst->pMonsterInfo.bBloodSplatOnDeath = src.pMonsterInfo.bBloodSplatOnDeath;
    dst->pMonsterInfo.uSpellSkillAndMastery1 = src.pMonsterInfo.uSpellSkillAndMastery1;
    dst->pMonsterInfo.uSpellSkillAndMastery2 = src.pMonsterInfo.uSpellSkillAndMastery2;
    dst->pMonsterInfo.field_3C_some_special_attack = src.pMonsterInfo.field_3C_some_special_attack;
    dst->pMonsterInfo.field_3E = src.pMonsterInfo.field_3E;
    dst->pMonsterInfo.uHP = src.pMonsterInfo.uHP;
    dst->pMonsterInfo.uAC = src.pMonsterInfo.uAC;
    dst->pMonsterInfo.uExp = src.pMonsterInfo.uExp;
    dst->pMonsterInfo.uBaseSpeed = src.pMonsterInfo.uBaseSpeed;
    dst->pMonsterInfo.uRecoveryTime = src.pMonsterInfo.uRecoveryTime;
    dst->pMonsterInfo.uAttackPreference = src.pMonsterInfo.uAttackPreference;
    dst->word_000084_range_attack = src.word_000084_range_attack;
    dst->word_000086_some_monster_id = src.word_000086_some_monster_id;  // base monster class monsterlist id
    dst->uActorRadius = src.uActorRadius;
    dst->uActorHeight = src.uActorHeight;
    dst->uMovementSpeed = src.uMovementSpeed;
    dst->vPosition = src.vPosition;
    dst->vVelocity = src.vVelocity;
    dst->uYawAngle = src.uYawAngle;
    dst->uPitchAngle = src.uPitchAngle;
    dst->uSectorID = src.uSectorID;
    dst->uCurrentActionLength = src.uCurrentActionLength;
    dst->vInitialPosition = src.vInitialPosition;
    dst->vGuardingPosition = src.vGuardingPosition;
    dst->uTetherDistance = src.uTetherDistance;
    dst->uAIState = static_cast<AIState>(src.uAIState);
    dst->uCurrentActionAnimation = static_cast<ActorAnimation>(src.uCurrentActionAnimation);
    dst->uCarriedItemID = ITEM_TYPE(src.uCarriedItemID);
    dst->field_B6 = src.field_B6;
    dst->field_B7 = src.field_B7;
    dst->uCurrentActionTime = src.uCurrentActionTime;

    Deserialize(src.pSpriteIDs, &dst->pSpriteIDs);
    Deserialize(src.pSoundSampleIDs, &dst->pSoundSampleIDs);
    Deserialize(src.pActorBuffs, &dst->pActorBuffs);
    Deserialize(src.ActorHasItems, &dst->ActorHasItems);

    dst->uGroup = src.uGroup;
    dst->uAlly = src.uAlly;

    Deserialize(src.pScheduledJobs, &dst->pScheduledJobs);

    dst->uSummonerID = src.uSummonerID;
    dst->uLastCharacterIDToHit = src.uLastCharacterIDToHit;
    dst->dword_000334_unique_name = src.dword_000334_unique_name;

    Deserialize(src.field_338, &dst->field_338);
}

void Serialize(const BLVDoor &src, BLVDoor_MM7 *dst) {
    memzero(dst);

    dst->uAttributes = std::to_underlying(src.uAttributes);
    dst->uDoorID = src.uDoorID;
    dst->uTimeSinceTriggered = src.uTimeSinceTriggered;
    dst->vDirection = src.vDirection;
    dst->uMoveLength = src.uMoveLength;
    dst->uOpenSpeed = src.uOpenSpeed;
    dst->uCloseSpeed = src.uCloseSpeed;
    dst->uNumVertices = src.uNumVertices;
    dst->uNumFaces = src.uNumFaces;
    dst->uNumSectors = src.uNumSectors;
    dst->uNumOffsets = src.uNumOffsets;
    dst->uState = std::to_underlying(src.uState);
    dst->field_4E = src.field_4E;
}

void Deserialize(const BLVDoor_MM7 &src, BLVDoor *dst) {
    dst->uAttributes = static_cast<DoorAttributes>(src.uAttributes);
    dst->uDoorID = src.uDoorID;
    dst->uTimeSinceTriggered = src.uTimeSinceTriggered;
    dst->vDirection = src.vDirection;
    dst->uMoveLength = src.uMoveLength;
    dst->uOpenSpeed = src.uOpenSpeed;
    dst->uCloseSpeed = src.uCloseSpeed;
    dst->uNumVertices = src.uNumVertices;
    dst->uNumFaces = src.uNumFaces;
    dst->uNumSectors = src.uNumSectors;
    dst->uNumOffsets = src.uNumOffsets;
    dst->uState = static_cast<BLVDoor::State>(src.uState);
    dst->field_4E = src.field_4E;
}

void Serialize(const BLVSector &src, BLVSector_MM7 *dst) {
    memzero(dst);

    dst->field_0 = src.field_0;
    dst->uNumFloors = src.uNumFloors;
    dst->field_6 = src.field_6;
    dst->uNumWalls = src.uNumWalls;
    dst->field_E = src.field_E;
    dst->uNumCeilings = src.uNumCeilings;
    dst->field_16 = src.field_16;
    dst->uNumFluids = src.uNumFluids;
    dst->field_1E = src.field_1E;
    dst->uNumPortals = src.uNumPortals;
    dst->field_26 = src.field_26;
    dst->uNumFaces = src.uNumFaces;
    dst->uNumNonBSPFaces = src.uNumNonBSPFaces;
    dst->uNumCylinderFaces = src.uNumCylinderFaces;
    dst->field_36 = src.field_36;
    dst->pCylinderFaces = src.pCylinderFaces;
    dst->uNumCogs = src.uNumCogs;
    dst->field_3E = src.field_3E;
    dst->uNumDecorations = src.uNumDecorations;
    dst->field_46 = src.field_46;
    dst->uNumMarkers = src.uNumMarkers;
    dst->field_4E = src.field_4E;
    dst->uNumLights = src.uNumLights;
    dst->field_56 = src.field_56;
    dst->uWaterLevel = src.uWaterLevel;
    dst->uMistLevel = src.uMistLevel;
    dst->uLightDistanceMultiplier = src.uLightDistanceMultiplier;
    dst->uMinAmbientLightLevel = src.uMinAmbientLightLevel;
    dst->uFirstBSPNode = src.uFirstBSPNode;
    dst->exit_tag = src.exit_tag;
    dst->pBounding = src.pBounding;
}

void Deserialize(const BLVSector_MM7 &src, BLVSector *dst) {
    dst->field_0 = src.field_0;
    dst->uNumFloors = src.uNumFloors;
    dst->field_6 = src.field_6;
    dst->uNumWalls = src.uNumWalls;
    dst->field_E = src.field_E;
    dst->uNumCeilings = src.uNumCeilings;
    dst->field_16 = src.field_16;
    dst->uNumFluids = src.uNumFluids;
    dst->field_1E = src.field_1E;
    dst->uNumPortals = src.uNumPortals;
    dst->field_26 = src.field_26;
    dst->uNumFaces = src.uNumFaces;
    dst->uNumNonBSPFaces = src.uNumNonBSPFaces;
    dst->uNumCylinderFaces = src.uNumCylinderFaces;
    dst->field_36 = src.field_36;
    dst->pCylinderFaces = src.pCylinderFaces;
    dst->uNumCogs = src.uNumCogs;
    dst->field_3E = src.field_3E;
    dst->uNumDecorations = src.uNumDecorations;
    dst->field_46 = src.field_46;
    dst->uNumMarkers = src.uNumMarkers;
    dst->field_4E = src.field_4E;
    dst->uNumLights = src.uNumLights;
    dst->field_56 = src.field_56;
    dst->uWaterLevel = src.uWaterLevel;
    dst->uMistLevel = src.uMistLevel;
    dst->uLightDistanceMultiplier = src.uLightDistanceMultiplier;
    dst->uMinAmbientLightLevel = src.uMinAmbientLightLevel;
    dst->uFirstBSPNode = src.uFirstBSPNode;
    dst->exit_tag = src.exit_tag;
    dst->pBounding = src.pBounding;
}

void Serialize(const GUICharMetric &src, GUICharMetric_MM7 *dst) {
    memzero(dst);
    dst->uLeftSpacing = src.uLeftSpacing;
    dst->uWidth = src.uWidth;
    dst->uRightSpacing = src.uRightSpacing;
}

void Deserialize(const GUICharMetric_MM7 &src, GUICharMetric *dst) {
    dst->uLeftSpacing = src.uLeftSpacing;
    dst->uWidth = src.uWidth;
    dst->uRightSpacing = src.uRightSpacing;
}

void Serialize(const FontData &src, FontData_MM7 *dst) {
    memzero(dst);

    dst->cFirstChar = src.cFirstChar;
    dst->cLastChar = src.cLastChar;
    dst->field_2 = src.field_2;
    dst->field_3 = src.field_3;
    dst->field_4 = src.field_4;
    dst->uFontHeight = src.uFontHeight;
    dst->field_7 = src.field_7;
    dst->palletes_count = src.palletes_count;

    Serialize(src.pMetrics, &dst->pMetrics);
    Serialize(src.font_pixels_offset, &dst->font_pixels_offset);

    std::copy(src.pFontData.begin(), src.pFontData.end(), dst->pFontData);
}

void Deserialize(const FontData_MM7 &src, size_t size, FontData *dst) {
    dst->cFirstChar = src.cFirstChar;
    dst->cLastChar = src.cLastChar;
    dst->field_2 = src.field_2;
    dst->field_3 = src.field_3;
    dst->field_4 = src.field_4;
    dst->uFontHeight = src.uFontHeight;
    dst->field_7 = src.field_7;
    dst->palletes_count = src.palletes_count;

    Deserialize(src.pMetrics, &dst->pMetrics);
    Deserialize(src.font_pixels_offset, &dst->font_pixels_offset);

    dst->pFontData.assign(src.pFontData, &src.pFontData[size - 4128]);
}

void Deserialize(const ODMFace_MM7 &src, ODMFace *dst) {
    dst->facePlane_old = src.facePlane;
    dst->facePlane.normal.x = dst->facePlane_old.normal.x / 65536.0;
    dst->facePlane.normal.y = dst->facePlane_old.normal.y / 65536.0;
    dst->facePlane.normal.z = dst->facePlane_old.normal.z / 65536.0;
    dst->facePlane.dist = dst->facePlane_old.dist / 65536.0;

    dst->zCalc.init(dst->facePlane_old);
    dst->uAttributes = FaceAttributes(src.attributes);
    dst->pVertexIDs = src.pVertexIDs;
    dst->pTextureUIDs = src.pTextureUIDs;
    dst->pTextureVIDs = src.pTextureVIDs;
    dst->pXInterceptDisplacements = src.pXInterceptDisplacements;
    dst->pYInterceptDisplacements = src.pYInterceptDisplacements;
    dst->pZInterceptDisplacements = src.pZInterceptDisplacements;
    dst->resource = nullptr;
    dst->sTextureDeltaU = src.sTextureDeltaU;
    dst->sTextureDeltaV = src.sTextureDeltaV;
    dst->pBoundingBox = src.pBoundingBox;
    dst->sCogNumber = src.sCogNumber;
    dst->sCogTriggeredID = src.sCogTriggeredID;
    dst->sCogTriggerType = src.sCogTriggerType;
    dst->field_128 = src.field_128;
    dst->field_129 = src.field_129;
    dst->uGradientVertex1 = src.uGradientVertex1;
    dst->uGradientVertex2 = src.uGradientVertex2;
    dst->uGradientVertex3 = src.uGradientVertex3;
    dst->uGradientVertex4 = src.uGradientVertex4;
    dst->uNumVertices = src.uNumVertices;
    dst->uPolygonType = static_cast<PolygonType>(src.uPolygonType);
    dst->uShadeType = src.uShadeType;
    dst->bVisible = src.bVisible;
    dst->field_132 = src.field_132;
    dst->field_133 = src.field_133;
}

void Deserialize(const SpawnPoint_MM7 &src, SpawnPoint *dst) {
    dst->vPosition = src.vPosition;
    dst->uRadius = src.uRadius;
    dst->uKind = static_cast<ObjectType>(src.uKind);
    if (dst->uKind == OBJECT_Actor) {
        dst->uItemIndex = ITEM_TREASURE_LEVEL_INVALID;
        dst->uMonsterIndex = src.uIndex;
    } else {
        Assert(dst->uKind == OBJECT_Item);
        dst->uItemIndex = static_cast<ITEM_TREASURE_LEVEL>(src.uIndex);
        dst->uMonsterIndex = 0;
    }
    dst->uAttributes = src.uAttributes;
    dst->uGroup = src.uGroup;
}

void Serialize(const SpriteObject &src, SpriteObject_MM7 *dst) {
    dst->uType = src.uType;
    dst->uObjectDescID = src.uObjectDescID;
    dst->vPosition = src.vPosition;
    dst->vVelocity = src.vVelocity;
    dst->uFacing = src.uFacing;
    dst->uSoundID = src.uSoundID;
    dst->uAttributes = std::to_underlying(src.uAttributes);
    dst->uSectorID = src.uSectorID;
    dst->uSpriteFrameID = src.uSpriteFrameID;
    dst->tempLifetime = src.tempLifetime;
    dst->field_22_glow_radius_multiplier = src.field_22_glow_radius_multiplier;
    Serialize(src.containing_item, &dst->containing_item);
    dst->uSpellID = std::to_underlying(src.uSpellID);
    dst->spell_level = src.spell_level;
    dst->spell_skill = std::to_underlying(src.spell_skill);
    dst->field_54 = src.field_54;
    dst->spell_caster_pid = src.spell_caster_pid;
    dst->spell_target_pid = src.spell_target_pid;
    dst->field_60_distance_related_prolly_lod = src.field_60_distance_related_prolly_lod;
    dst->field_61 = std::to_underlying(src.field_61);
    dst->field_62[0] = src.field_62[0];
    dst->field_62[1] = src.field_62[1];
    dst->initialPosition = src.initialPosition;
}

void Deserialize(const SpriteObject_MM7 &src, SpriteObject *dst) {
    dst->uType = static_cast<SPRITE_OBJECT_TYPE>(src.uType);
    dst->uObjectDescID = src.uObjectDescID;
    dst->vPosition = src.vPosition;
    dst->vVelocity = src.vVelocity;
    dst->uFacing = src.uFacing;
    dst->uSoundID = src.uSoundID;
    dst->uAttributes = static_cast<SPRITE_ATTRIBUTES>(src.uAttributes);
    dst->uSectorID = src.uSectorID;
    dst->uSpriteFrameID = src.uSpriteFrameID;
    dst->tempLifetime = src.tempLifetime;
    dst->field_22_glow_radius_multiplier = src.field_22_glow_radius_multiplier;
    Deserialize(src.containing_item, &dst->containing_item);
    dst->uSpellID = static_cast<SPELL_TYPE>(src.uSpellID);
    dst->spell_level = src.spell_level;
    dst->spell_skill = static_cast<PLAYER_SKILL_MASTERY>(src.spell_skill);
    dst->field_54 = src.field_54;
    dst->spell_caster_pid = src.spell_caster_pid;
    dst->spell_target_pid = src.spell_target_pid;
    dst->field_60_distance_related_prolly_lod = src.field_60_distance_related_prolly_lod;
    dst->field_61 = static_cast<ABILITY_INDEX>(src.field_61);
    dst->field_62[0] = src.field_62[0];
    dst->field_62[1] = src.field_62[1];
    dst->initialPosition = src.initialPosition;
}
