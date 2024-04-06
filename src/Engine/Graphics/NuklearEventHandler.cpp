#include "NuklearEventHandler.h"

#include <nuklear_config.h>

#include "Nuklear.h"

NuklearEventHandler::NuklearEventHandler() : PlatformEventFilter(EVENTS_ALL) {}

bool NuklearEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    PlatformKey key = event->key;
    PlatformModifiers mods = event->mods;

    if (nuklear->KeyEvent(key))
        return true;

    return KeyEvent(key, mods, true);
}

bool NuklearEventHandler::keyReleaseEvent(const PlatformKeyEvent *event) {
    PlatformKey key = event->key;
    PlatformModifiers mods = event->mods;

    if (key == PlatformKey::KEY_TILDE && (mods & MOD_CTRL))
        nuklear->Reload();

    return KeyEvent(key, mods, false);
}

bool NuklearEventHandler::KeyEvent(PlatformKey key, PlatformModifiers mods, bool down) {
    if (key == PlatformKey::KEY_SHIFT) {
        nk_input_key(nuklear->ctx, NK_KEY_SHIFT, down);
    } else if (key == PlatformKey::KEY_DELETE) {
        nk_input_key(nuklear->ctx, NK_KEY_DEL, down);
    } else if (key == PlatformKey::KEY_RETURN) {
        nk_input_key(nuklear->ctx, NK_KEY_ENTER, down);
    } else if (key == PlatformKey::KEY_TAB) {
        nk_input_key(nuklear->ctx, NK_KEY_TAB, down);
    } else if (key == PlatformKey::KEY_BACKSPACE) {
        nk_input_key(nuklear->ctx, NK_KEY_BACKSPACE, down);
    } else if (key == PlatformKey::KEY_HOME) {
        nk_input_key(nuklear->ctx, NK_KEY_TEXT_START, down);
        nk_input_key(nuklear->ctx, NK_KEY_SCROLL_START, down);
    } else if (key == PlatformKey::KEY_END) {
        nk_input_key(nuklear->ctx, NK_KEY_TEXT_END, down);
        nk_input_key(nuklear->ctx, NK_KEY_SCROLL_END, down);
    } else if (key == PlatformKey::KEY_PAGEDOWN) {
        nk_input_key(nuklear->ctx, NK_KEY_SCROLL_DOWN, down);
    } else if (key == PlatformKey::KEY_PAGEUP) {
        nk_input_key(nuklear->ctx, NK_KEY_SCROLL_UP, down);
    } else if (key == PlatformKey::KEY_Z && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_TEXT_UNDO, down);
    } else if (key == PlatformKey::KEY_R && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_TEXT_REDO, down);
    } else if (key == PlatformKey::KEY_C && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_COPY, down);
    } else if (key == PlatformKey::KEY_V && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_PASTE, down);
    } else if (key == PlatformKey::KEY_X && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_CUT, down);
    } else if (key == PlatformKey::KEY_B && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_TEXT_LINE_START, down);
    } else if (key == PlatformKey::KEY_E && (mods & MOD_CTRL)) {
        nk_input_key(nuklear->ctx, NK_KEY_TEXT_LINE_END, down);
    } else if (key == PlatformKey::KEY_UP) {
        nk_input_key(nuklear->ctx, NK_KEY_UP, down);
    } else if (key == PlatformKey::KEY_DOWN) {
        nk_input_key(nuklear->ctx, NK_KEY_DOWN, down);
    } else if (key == PlatformKey::KEY_LEFT) {
        if (mods & MOD_CTRL)
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_WORD_LEFT, down);
        else
            nk_input_key(nuklear->ctx, NK_KEY_LEFT, down);
    } else if (key == PlatformKey::KEY_RIGHT) {
        if (mods & MOD_CTRL)
            nk_input_key(nuklear->ctx, NK_KEY_TEXT_WORD_RIGHT, down);
        else
            nk_input_key(nuklear->ctx, NK_KEY_RIGHT, down);
    }
    return nk_item_is_any_active(nuklear->ctx);
}

bool NuklearEventHandler::mouseMoveEvent(const PlatformMouseEvent *event) {
    nk_input_motion(nuklear->ctx, event->pos.x, event->pos.y);
    return nk_item_is_any_active(nuklear->ctx);
}

bool NuklearEventHandler::mousePressEvent(const PlatformMouseEvent *event) {
    if (event->button == BUTTON_LEFT && event->isDoubleClick)
        nk_input_button(nuklear->ctx, NK_BUTTON_DOUBLE, event->pos.x, event->pos.y, true);

    return MouseEvent(event->button, event->pos, true);
}

bool NuklearEventHandler::mouseReleaseEvent(const PlatformMouseEvent *event) {
    return MouseEvent(event->button, event->pos, false);
}

bool NuklearEventHandler::MouseEvent(PlatformMouseButton button, const Pointi &pos, bool down) {
    /* mouse button */
    if (button == BUTTON_LEFT) {
        nk_input_button(nuklear->ctx, NK_BUTTON_LEFT, pos.x, pos.y, down);
    } else if (button == BUTTON_MIDDLE) {
        nk_input_button(nuklear->ctx, NK_BUTTON_MIDDLE, pos.x, pos.y, down);
    } else if (button == BUTTON_RIGHT) {
        nk_input_button(nuklear->ctx, NK_BUTTON_RIGHT, pos.x, pos.y, down);
    }
    return nk_item_is_any_active(nuklear->ctx);
}

bool NuklearEventHandler::wheelEvent(const PlatformWheelEvent *event) {
    nk_input_scroll(nuklear->ctx, nk_vec2(event->angleDelta.x, event->angleDelta.y));
    return nk_item_is_any_active(nuklear->ctx);
}

bool NuklearEventHandler::textInputEvent(const PlatformTextInputEvent *event) {
    nk_glyph glyph;
    memcpy(glyph, event->text, NK_UTF_SIZE);
    nk_input_glyph(nuklear->ctx, glyph);
    return nk_item_is_any_active(nuklear->ctx);
}
