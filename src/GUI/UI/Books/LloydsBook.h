#pragma once

#include "Engine/Pid.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Spells/CastSpellInfo.h"

#include "GUI/UI/UIBooks.h"

class GUIWindow_LloydsBook : public GUIWindow_Book {
 public:
    GUIWindow_LloydsBook(Pid casterPid, SpellCastFlags castFlags);
    virtual ~GUIWindow_LloydsBook() {}

    virtual void Update() override;

    void flipButtonClicked(bool isRecalling);
    void hintBeaconSlot(int beaconId);
    void installOrRecallBeacon(int beaconId);

 private:
    const Pid _casterPid;
    const SpellCastFlags _castFlags;
    int _maxBeacons;
    bool _recallingBeacon;
    int _spellLevel;
    CharacterSkillMastery _waterMastery;
};
