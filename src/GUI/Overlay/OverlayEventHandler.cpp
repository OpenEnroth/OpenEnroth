#include "OverlayEventHandler.h"

#include <imgui/backends/imgui_impl_sdl2.h>
#include <nuklear_config.h>
#include <imgui/imgui.h>
#include <cstring>

OverlayEventHandler::OverlayEventHandler(struct nk_context *context)
    : PlatformEventFilter(EVENTS_ALL)
    , _context(context) {
}

bool OverlayEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    return keyEvent(event->key, event->mods, true);
}

bool OverlayEventHandler::keyReleaseEvent(const PlatformKeyEvent *event) {
    return keyEvent(event->key, event->mods, false);
}

bool OverlayEventHandler::keyEvent(PlatformKey key, PlatformModifiers mods, bool keyPressed) {
    if (key == PlatformKey::KEY_SHIFT) {
        nk_input_key(_context, NK_KEY_SHIFT, keyPressed);
    } else if (key == PlatformKey::KEY_DELETE) {
        nk_input_key(_context, NK_KEY_DEL, keyPressed);
    } else if (key == PlatformKey::KEY_RETURN) {
        nk_input_key(_context, NK_KEY_ENTER, keyPressed);
    } else if (key == PlatformKey::KEY_TAB) {
        nk_input_key(_context, NK_KEY_TAB, keyPressed);
    } else if (key == PlatformKey::KEY_BACKSPACE) {
        nk_input_key(_context, NK_KEY_BACKSPACE, keyPressed);
    } else if (key == PlatformKey::KEY_HOME) {
        nk_input_key(_context, NK_KEY_TEXT_START, keyPressed);
        nk_input_key(_context, NK_KEY_SCROLL_START, keyPressed);
    } else if (key == PlatformKey::KEY_END) {
        nk_input_key(_context, NK_KEY_TEXT_END, keyPressed);
        nk_input_key(_context, NK_KEY_SCROLL_END, keyPressed);
    } else if (key == PlatformKey::KEY_PAGEDOWN) {
        nk_input_key(_context, NK_KEY_SCROLL_DOWN, keyPressed);
    } else if (key == PlatformKey::KEY_PAGEUP) {
        nk_input_key(_context, NK_KEY_SCROLL_UP, keyPressed);
    } else if (key == PlatformKey::KEY_Z && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_TEXT_UNDO, keyPressed);
    } else if (key == PlatformKey::KEY_R && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_TEXT_REDO, keyPressed);
    } else if (key == PlatformKey::KEY_C && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_COPY, keyPressed);
    } else if (key == PlatformKey::KEY_V && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_PASTE, keyPressed);
    } else if (key == PlatformKey::KEY_X && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_CUT, keyPressed);
    } else if (key == PlatformKey::KEY_B && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_TEXT_LINE_START, keyPressed);
    } else if (key == PlatformKey::KEY_E && (mods & MOD_CTRL)) {
        nk_input_key(_context, NK_KEY_TEXT_LINE_END, keyPressed);
    } else if (key == PlatformKey::KEY_UP) {
        nk_input_key(_context, NK_KEY_UP, keyPressed);
    } else if (key == PlatformKey::KEY_DOWN) {
        nk_input_key(_context, NK_KEY_DOWN, keyPressed);
    } else if (key == PlatformKey::KEY_LEFT) {
        if (mods & MOD_CTRL)
            nk_input_key(_context, NK_KEY_TEXT_WORD_LEFT, keyPressed);
        else
            nk_input_key(_context, NK_KEY_LEFT, keyPressed);
    } else if (key == PlatformKey::KEY_RIGHT) {
        if (mods & MOD_CTRL)
            nk_input_key(_context, NK_KEY_TEXT_WORD_RIGHT, keyPressed);
        else
            nk_input_key(_context, NK_KEY_RIGHT, keyPressed);
    }
    return _context->last_widget_state & NK_WIDGET_STATE_MODIFIED ||
        _context->text_edit.active || (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard);
}

bool OverlayEventHandler::mouseMoveEvent(const PlatformMouseEvent *event) {
    nk_input_motion(_context, event->pos.x, event->pos.y);
    return _context->last_widget_state & NK_WIDGET_STATE_MODIFIED;
}

bool OverlayEventHandler::mousePressEvent(const PlatformMouseEvent *event) {
    if (event->button == BUTTON_LEFT && event->isDoubleClick)
        nk_input_button(_context, NK_BUTTON_DOUBLE, event->pos.x, event->pos.y, true);

    return mouseEvent(event->button, event->pos, true);
}

bool OverlayEventHandler::mouseReleaseEvent(const PlatformMouseEvent *event) {
    return mouseEvent(event->button, event->pos, false);
}

bool OverlayEventHandler::mouseEvent(PlatformMouseButton button, const Pointi &pos, bool down) {
    /* mouse button */
    if (button == BUTTON_LEFT) {
        nk_input_button(_context, NK_BUTTON_LEFT, pos.x, pos.y, down);
    } else if (button == BUTTON_MIDDLE) {
        nk_input_button(_context, NK_BUTTON_MIDDLE, pos.x, pos.y, down);
    } else if (button == BUTTON_RIGHT) {
        nk_input_button(_context, NK_BUTTON_RIGHT, pos.x, pos.y, down);
    }
    return nk_item_is_any_active(_context) || (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse);
}

bool OverlayEventHandler::wheelEvent(const PlatformWheelEvent *event) {
    nk_input_scroll(_context, nk_vec2(event->angleDelta.x, event->angleDelta.y));
    return nk_item_is_any_active(_context) || (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse);
}

bool OverlayEventHandler::textInputEvent(const PlatformTextInputEvent *event) {
    nk_glyph glyph;
    memcpy(glyph, event->text.c_str(), NK_UTF_SIZE);
    nk_input_glyph(_context, glyph);
    return nk_item_is_any_active(_context);
}

bool OverlayEventHandler::nativeEvent(const PlatformNativeEvent *event) {
    // Here we're assuming the native event is coming from SDL
    const SDL_Event *sdlEvent = reinterpret_cast<const SDL_Event *>(event->nativeEvent);
    ImGui_ImplSDL2_ProcessEvent(sdlEvent);
    return false;
}
