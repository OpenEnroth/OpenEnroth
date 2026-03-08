#include "GameTestCommon.h"

#include <cassert>

#include "Engine/Engine.h"
#include "Engine/Party.h"

void prepareForBattleTest() {
    assert(engine->_currentLoadedMapId == MAP_EMERALD_ISLAND);

    // Move party in front of the bridge.
    pParty->pos = Vec3f(12552, 2000, 1);

    // Wizard's eye is handy for debugging.
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(tomorrow, MASTERY_GRANDMASTER, 30, 0, 0);

    // Make sure only the 1st char is alive.
    for (int i = 1; i < 4; i++)
        pParty->pCharacters[i].SetCondDeadWithBlockCheck(false);

    // We want char0 chonky.
    Character &char0 = pParty->pCharacters[0];
    char0.sLevelModifier = 400;
    char0.health = pParty->pCharacters[0].GetMaxHealth();
    char0._stats[ATTRIBUTE_LUCK] = 0; // We don't want luck rolls that decrease damage dealt.
}
