#pragma once

#include "Engine/Time.h"

#include "Engine/Objects/CharacterEnums.h"

// TODO(pskelton): style
struct SpellBuff {
    /**
     * @offset 0x4584E0
     */
    bool Apply(GameTime time, CharacterSkillMastery uSkillMastery,
               int uPower, int uOverlayID, uint8_t caster);

    /**
     * @offset 0x458585
     */
    void Reset();

    /**
     * @offset 0x4585CA
     */
    bool IsBuffExpiredToTime(GameTime time);

    /**
     * @offset 0x42EB31
     * Active is state where spell buff is in effect
     */
    bool Active() const { return this->expireTime.Valid(); }
    /**
    * Inactive is state where spell buff is not in effect (includes state expired)
    */
    bool Inactive() const { return !Active(); }
    /**
    * Expired is state where spell buff is not in effect after previously being active
    */
    bool Expired() const { return this->expireTime.Expired(); }
    GameTime &GetExpireTime() { return this->expireTime; }

    GameTime expireTime;
    uint16_t power = 0; // Spell power, semantics are spell-specific.
    CharacterSkillMastery skillMastery = CHARACTER_SKILL_MASTERY_NONE; // 1-4, normal to grandmaster.
    uint16_t overlayID = 0;
    uint8_t caster = 0;
    bool isGMBuff = false; // Buff was casted at grandmaster mastery
};
