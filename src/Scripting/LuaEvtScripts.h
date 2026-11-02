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

    virtual void loadGlobalScripts() override;
    virtual void loadMapScripts(std::string_view mapName) override;
    virtual bool hasEvent(EvtSource source, int eventId) const override;
    virtual EvtTransition runEvent(const EvtContext &context) override;
    virtual EvtTransition resumeEvent(int frame) override;
    virtual EvtTransition fireTimer(int handle) override;
    virtual std::optional<std::string> eventHint(int eventId) const override;
    virtual std::optional<bool> canShowTopic(int eventId) override;
    virtual EvtTransition onMapLoad() override;
    virtual void onMapLeave() override;

 private:
    /**
     * @param function                  Function of `mmext.core` to call.
     * @param args                      Its arguments.
     * @return                          What the function returned, or nil if it failed.
     */
    template<class... Args>
    sol::object call(std::string_view function, Args &&... args) const;

    /**
     * @param function                  Function of `mmext.core` to call, one that returns whether a map transition started.
     * @param args                      Its arguments.
     * @return                          What the function returned.
     */
    template<class... Args>
    EvtTransition callForTransition(std::string_view function, Args &&... args) const;

 private:
    sol::table _core;
};
