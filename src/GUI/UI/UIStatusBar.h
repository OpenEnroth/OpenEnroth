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
    void drawForced(const std::string &str, Color color);
    void update();
    void clearAll();

    void setPermanent(const std::string &str);
    void clearPermanent();

    void setEvent(const std::string &str);
    void setEventShort(const std::string &str);
    void clearEvent();

    template<class... Args> void setPermanent(int locId, Args &&... args) {
        setPermanent(localization->FormatString(locId, std::forward<Args>(args)...));
    }

    template<class... Args> void setEvent(int locId, Args &&... args) {
        setEvent(localization->FormatString(locId, std::forward<Args>(args)...));
    }

    template<class... Args> void setEventShort(int locId, Args &&... args) {
        setEventShort(localization->FormatString(locId, std::forward<Args>(args)...));
    }

    void nothingHere();

 private:
    std::string _statusString = "";
    std::string _eventStatusString = "";
    int _eventStatusExpireTime = 0;
};
