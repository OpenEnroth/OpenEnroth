#pragma once

#include <string>

#include "Engine/Evt/EvtContinuation.h"
#include "Engine/MapEnums.h"
#include "Engine/PartyPlacement.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Transition : public GUIWindow {
 public:
    GUIWindow_Transition(WindowType windowType, ScreenType screenType);
    virtual ~GUIWindow_Transition();

 protected:
    void createButtons(const std::string &okHint, const std::string &cancelHint, UIMessageType confirmMsg, UIMessageType cancelMsg);

    GraphicsImage *_dialogueBackground = nullptr;
};

class GUIWindow_Travel : public GUIWindow_Transition {
 public:
    GUIWindow_Travel();
    virtual ~GUIWindow_Travel() {}

    virtual void Update() override;
};

class GUIWindow_IndoorEntryExit : public GUIWindow_Transition {
 public:
    GUIWindow_IndoorEntryExit(HouseId transitionHouse, uint32_t exit_pic_id, const MapDestination &destination, std::string_view locationName,
                              EvtContinuation continuation);
    virtual ~GUIWindow_IndoorEntryExit() {}

    virtual void Update() override;

    /**
     * @return                          Where the party goes if the player confirms.
     */
    [[nodiscard]] const MapDestination &destination() const { return _destination; }

    /**
     * @return                          Event paused on this dialogue, to resume if the party stays where it is.
     */
    [[nodiscard]] EvtContinuation continuation() const { return _continuation; }

    std::string _mapName = "";
    int _transitionStringId = 0;

 private:
    MapDestination _destination;
    EvtContinuation _continuation;
};
