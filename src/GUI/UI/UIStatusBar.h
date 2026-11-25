#pragma once

#include <string>
#include <utility>

#include "Engine/Localization.h"

#include "Library/Color/Color.h"

class StatusBar {
 public:
    static const int EVENT_DURATION = 2000; // 2s
    static const int EVENT_DURATION_SHORT = 500; // 0.5s

    const std::string &get();
    // The most recent event line, kept past its on-screen expiry, so a test can read it.
    const std::string &lastEvent() const { return _lastEvent; }
    // Bumps on every event line set, so a headless reader can tell "the game said this
    // just now" from "this is still the line from ten rounds ago" (`lastEvent` keeps the text forever).
    int lastEventSeq() const { return _lastEventSeq; }
    // Restores a captured (text, seq) pair. For tests.
    void restoreLastEvent(std::string_view text, int seq) { _lastEvent = text; _lastEventSeq = seq; }
    void draw();
    void drawForced(std::string_view str, Color color);
    void update();
    void clearAll();

    void setPermanent(std::string_view str);
    void clearPermanent();

    void setEvent(std::string_view str);
    void setEventShort(std::string_view str);
    void clearEvent();

    template<class... Args> void setPermanent(LstrId locId, Args &&... args) {
        setPermanent(localization->format(locId, std::forward<Args>(args)...));
    }

    template<class... Args> void setEvent(LstrId locId, Args &&... args) {
        setEvent(localization->format(locId, std::forward<Args>(args)...));
    }

    template<class... Args> void setEventShort(LstrId locId, Args &&... args) {
        setEventShort(localization->format(locId, std::forward<Args>(args)...));
    }

    // TODO(captainurist): this only shows in some cases. Clicking the sky, terrain, a far away face or a building wall
    // is silent, and so is Space with nothing in reach. Make it consistent.
    void nothingHere();

 private:
    std::string _statusString = "";
    std::string _eventStatusString = "";
    std::string _lastEvent = "";
    int _lastEventSeq = 0;
    int _eventStatusExpireTime = 0;
};
