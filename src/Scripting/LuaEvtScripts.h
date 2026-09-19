#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <sol/sol.hpp>

#include "Engine/Evt/EvtScripts.h"

/**
 * Event handlers written in Lua against MMExtension's `evt` API. The API itself lives in `scripts/mmext/core.lua`,
 * this class forwards the engine's calls to it.
 */
class LuaEvtScripts : public EvtScripts {
 public:
    explicit LuaEvtScripts(sol::state_view lua);
    virtual ~LuaEvtScripts();

    virtual void loadGlobalScripts() override;
    virtual void loadMapScripts(std::string_view mapName) override;
    virtual bool hasEvent(bool isGlobal, int eventId) const override;
    virtual bool runEvent(bool isGlobal, int eventId, Pid targetObj, bool canShowMessages) override;
    virtual bool resumeEvent(int eventId, bool *mapExitTriggered) override;
    virtual void cancelEvent() override;
    virtual std::optional<std::string> eventHint(int eventId) const override;
    virtual std::optional<bool> canShowTopic(int eventId) override;
    virtual bool onMapLoad() override;
    virtual void onMapLeave() override;

 private:
    /**
     * @param function                  Function of `mmext.core` to call.
     * @param args                      Its arguments.
     * @return                          What the function returned, or nil if it failed.
     */
    template<class... Args>
    sol::object call(std::string_view function, Args &&... args) const;

 private:
    sol::table _core;
};
