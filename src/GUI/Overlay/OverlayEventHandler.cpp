#include "OverlayEventHandler.h"

#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>

#include "Engine/Engine.h"

OverlayEventHandler::OverlayEventHandler() : PlatformEventFilter(EVENTS_ALL) {
}

bool OverlayEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    return keyEvent(event->key, event->mods, true);
}

bool OverlayEventHandler::keyReleaseEvent(const PlatformKeyEvent *event) {
    return keyEvent(event->key, event->mods, false);
}

bool OverlayEventHandler::keyEvent(PlatformKey key, PlatformModifiers mods, bool keyPressed) {
    if (key == engine->config->keybindings.Console.value() || key == engine->config->gamepad.Console.value())
        return false; // Pass close console keys to the game to handle.

    return ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
}

bool OverlayEventHandler::mousePressEvent(const PlatformMouseEvent *event) {
    return mouseEvent(event->button, event->pos, true);
}

bool OverlayEventHandler::mouseReleaseEvent(const PlatformMouseEvent *event) {
    return mouseEvent(event->button, event->pos, false);
}

bool OverlayEventHandler::mouseEvent(PlatformMouseButton button, const Pointi &pos, bool down) {
    return ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
}

bool OverlayEventHandler::wheelEvent(const PlatformWheelEvent *event) {
    return ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
}

bool OverlayEventHandler::nativeEvent(const PlatformNativeEvent *event) {
    // Here we're assuming the native event is coming from SDL
    const SDL_Event *sdlEvent = static_cast<const SDL_Event *>(event->nativeEvent);
    ImGui_ImplSDL2_ProcessEvent(sdlEvent);
    return false;
}
