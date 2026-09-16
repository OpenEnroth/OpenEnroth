#pragma once

#include "Engine/Objects/CharacterEnums.h"

#include "Core/Time/Time.h"

// TODO(pskelton): style
struct SpellBuff {
    /**
     * @offset 0x4584E0
     * TODO(pskelton): check for inconsistent use of caster
     * caster is the 0-based party character index, or -1 for an external source like a potion or an npc
     */
    bool Apply(Time time, Mastery uSkillMastery,
               int uPower, int uOverlayID, int caster);

    /**
     * @offset 0x458585
     */
    void Reset();

    /**
     * @offset 0x4585CA
     */
    bool IsBuffExpiredToTime(Time time);

    /**
     * @offset 0x42EB31
     * Active is state where spell buff is in effect
     */
    bool Active() const { return this->expireTime.isValid(); }
    /**
    * Inactive is state where spell buff is not in effect (includes state expired)
    */
    bool Inactive() const { return !Active(); }
    /**
    * Expired is state where spell buff is not in effect after previously being active
    */
    bool Expired() const { return this->expireTime.Expired(); }
    Time &GetExpireTime() { return this->expireTime; }

    Time expireTime;
    uint16_t power = 0; // Spell power, semantics are spell-specific.
    Mastery skillMastery = MASTERY_NONE;
    uint16_t overlayId = 0;
    int caster = -1; // -1 when the buff came from a potion or an npc.
    bool isGM = false; // Buff was casted at grandmaster mastery
};
