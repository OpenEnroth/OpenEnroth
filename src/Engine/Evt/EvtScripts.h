#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "Engine/Evt/EvtContext.h"
#include "Engine/Evt/EvtEnums.h"

/**
 * Event handlers written in a scripting language, which run next to the events of the evt files. The engine talks to
 * them through this interface and knows nothing of the language behind it.
 *
 * An evt event runs first and the scripted handlers for the same id run after it. A script that replaces an event
 * removes it from the `EvtProgram` when the map's scripts are loaded.
 */
class EvtScripts {
 public:
    virtual ~EvtScripts() = default;

    /**
     * Called when a game is started or loaded, right after the global evt file is loaded.
     */
    virtual void loadGlobalScripts() = 0;

    /**
     * Called when a map is loaded, before its evt triggers are registered. Drops the scripts of the previous map.
     *
     * @param mapName                   File name of the map without the extension, e.g. "d27".
     */
    virtual void loadMapScripts(std::string_view mapName) = 0;

    /**
     * @param source                    Evt file that the event belongs to.
     * @param eventId                   Event id.
     * @return                          Whether a scripted handler exists for the event.
     */
    [[nodiscard]] virtual bool hasEvent(EvtSource source, int eventId) const = 0;

    /**
     * @param context                   Event to run the handlers of.
     * @return                          Whether a handler started a map transition.
     */
    [[nodiscard]] virtual EvtTransition runEvent(const EvtContext &context) = 0;

    /**
     * Continues a handler that stopped to wait for a dialogue.
     *
     * @param frame                     Handler to continue, see `EvtContext::scriptFrame`.
     * @return                          Whether the handler started a map transition.
     */
    [[nodiscard]] virtual EvtTransition resumeEvent(int frame) = 0;

    /**
     * @param handle                    Timer that is due, as the scripts passed it to `addTimer`.
     * @return                          Whether its handler started a map transition.
     */
    [[nodiscard]] virtual EvtTransition fireTimer(int handle) = 0;

    /**
     * @param eventId                   Event id.
     * @return                          Hint that the scripts set for the event, if any.
     */
    [[nodiscard]] virtual std::optional<std::string> eventHint(int eventId) const = 0;

    /**
     * @param eventId                   Id of a global event that stands behind an NPC topic.
     * @return                          Whether to show the topic, if the scripts have a say in it.
     */
    [[nodiscard]] virtual std::optional<bool> canShowTopic(int eventId) = 0;

    /**
     * Called after `loadMapScripts`, once the evt triggers are registered and the evt map load events have run.
     * Runs the scripts' map load handlers, and fires their timers that fire at once.
     *
     * @return                          Whether a map load handler started a map transition.
     */
    [[nodiscard]] virtual EvtTransition onMapLoad() = 0;

    virtual void onMapLeave() = 0;
};
