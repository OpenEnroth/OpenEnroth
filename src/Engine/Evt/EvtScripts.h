#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "Engine/Pid.h"

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
     * @param isGlobal                  Whether to look at the global events.
     * @param eventId                   Event id.
     * @return                          Whether a scripted handler exists for the event.
     */
    virtual bool hasEvent(bool isGlobal, int eventId) const = 0;

    /**
     * @param isGlobal                  Whether to run a global event.
     * @param eventId                   Event id.
     * @param targetObj                 Object that triggered the event.
     * @param canShowMessages           Whether the event can show status texts and open dialogues.
     * @return                          Whether the event sent the party to another map.
     */
    virtual bool runEvent(bool isGlobal, int eventId, Pid targetObj, bool canShowMessages) = 0;

    /**
     * Continues a handler that stopped to wait for a dialogue.
     *
     * @param eventId                   Event id.
     * @param[out] mapExitTriggered     Whether the event sent the party to another map.
     * @return                          Whether a handler of this event was waiting.
     */
    virtual bool resumeEvent(int eventId, bool *mapExitTriggered) = 0;

    /**
     * @param eventId                   Event id.
     * @return                          Hint that the scripts set for the event, if any.
     */
    virtual std::optional<std::string> eventHint(int eventId) const = 0;

    /**
     * @param eventId                   Id of a global event that stands behind an NPC topic.
     * @return                          Whether to show the topic, if the scripts have a say in it.
     */
    virtual std::optional<bool> canShowTopic(int eventId) = 0;

    /**
     * Called after `loadMapScripts`, once the evt triggers are registered and the evt map load events have run.
     * Registers the scripts' timers and runs their map load handlers.
     */
    virtual void onMapLoad() = 0;

    virtual void onMapLeave() = 0;
};
