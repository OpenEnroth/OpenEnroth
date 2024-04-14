#include "RmlEventListenerInstancer.h"

#include <Engine/Engine.h>
#include <Media/Audio/AudioPlayer.h>
#include <GUI/GUIWindow.h>

EventListenerCallback::EventListenerCallback(std::function<void(Rml::Event &)> callback) : _callback(callback) {
}

void EventListenerCallback::ProcessEvent(Rml::Event &event) {
    _callback(event);
}

void EventListenerCallback::OnAttach(Rml::Element *element) {
}

void EventListenerCallback::OnDetach(Rml::Element *element) {
}

Rml::EventListener *RmlEventListenerInstancer::InstanceEventListener(const Rml::String &value, Rml::Element *element) {
    return new EventListenerCallback([value](Rml::Event &event) {
        if (event.GetType() == "click") {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
        }
        if (value == "game.new()") {
            SetCurrentMenuID(MENU_NEWGAME);
        } else if (value == "game.load()") {
            SetCurrentMenuID(MENU_SAVELOAD);
        } else if (value == "game.credits()") {
            SetCurrentMenuID(MENU_CREDITS);
        } else if (value == "game.exit()") {
            SetCurrentMenuID(MENU_EXIT_GAME);
        }
    });
    return nullptr;
}
