#include "GameTestCommon.h"

#include <cassert>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Party.h"

static int faceForRace(Race race) {
    switch (race) {
    default: assert(false); [[fallthrough]];
    case RACE_HUMAN:  return 0;
    case RACE_ELF:    return 8;
    case RACE_DWARF:  return 12;
    case RACE_GOBLIN: return 17; // Default char0 face - keep voice & portrait stable for the no-arg case.
    }
}

void prepareForBattleTest(const std::vector<CharacterPreset> &presets) {
    assert(presets.size() >= 1 && presets.size() <= pParty->pCharacters.size());
    assert(engine->_currentLoadedMapId == MAP_EMERALD_ISLAND);

    // Move party in front of the bridge.
    pParty->pos = Vec3f(12552, 2000, 1);

    // Wizard's eye is handy for debugging.
    Time tomorrow = pParty->GetPlayingTime() + Duration::fromDays(1);
    pParty->pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Apply(tomorrow, MASTERY_GRANDMASTER, 30, 0, 0);

    // Apply class & race to the configured chars; portrait & voice follow the race.
    for (int i = 0; i < presets.size(); ++i) {
        Character &c = pParty->pCharacters[i];
        c.ChangeClass(presets[i].classType);
        c.uCurrentFace = faceForRace(presets[i].race);
        c.uPrevFace = c.uCurrentFace;
        c.uVoiceID = c.uCurrentFace;
        c.uPrevVoiceID = c.uCurrentFace;
        c.uSex = c.GetSexByVoice();
    }

    // Kill off characters past the configured set.
    for (int i = presets.size(); i < pParty->pCharacters.size(); ++i)
        pParty->pCharacters[i].SetCondDeadWithBlockCheck(false);

    // Chonk each configured character so they survive incoming attacks.
    for (int i = 0; i < presets.size(); ++i) {
        Character &c = pParty->pCharacters[i];
        c.sLevelModifier = 5000;
        c._stats[ATTRIBUTE_ENDURANCE] = 500;
        c.setSkillValue(SKILL_BODYBUILDING, CombinedSkillValue(63, MASTERY_GRANDMASTER));
        c.health = c.GetMaxHealth();
        c._stats[ATTRIBUTE_LUCK] = 0; // We don't want luck rolls that decrease damage dealt.
    }
}
