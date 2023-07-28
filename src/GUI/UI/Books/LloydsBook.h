#pragma once
#include "GUI/UI/UIBooks.h"

class GUIWindow_LloydsBook : public GUIWindow_Book {
 public:
    GUIWindow_LloydsBook(int casterId, int spellLevel);
    virtual ~GUIWindow_LloydsBook() {}

    virtual void Update() override;

    void flipButtonClicked(bool isRecalling);
    void hintBeaconSlot(int beaconId);
    void installOrRecallBeacon(int beaconId);

 private:
    int _maxBeacons;
    bool _recallingBeacon;
    int _casterId;
    int _spellLevel;
};
