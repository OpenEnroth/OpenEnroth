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
        setPermanent(localization->FormatString(locId, std::forward<Args>(args)...));
    }

    template<class... Args> void setEvent(LstrId locId, Args &&... args) {
        setEvent(localization->FormatString(locId, std::forward<Args>(args)...));
    }

    template<class... Args> void setEventShort(LstrId locId, Args &&... args) {
        setEventShort(localization->FormatString(locId, std::forward<Args>(args)...));
    }

    void nothingHere();

 private:
    std::string _statusString = "";
    std::string _eventStatusString = "";
    int _eventStatusExpireTime = 0;
};
