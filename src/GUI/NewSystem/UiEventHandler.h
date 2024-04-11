#pragma once

#include <RmlUi/Core/Input.h>
#include <functional>

#include "Library/Platform/Filters/PlatformEventFilter.h"

namespace Rml {
class Context;
}

class Renderer;

typedef std::function<Rml::Context *()> GetMainContextFunc;

class UiEventHandler : public PlatformEventFilter {
 public:
    explicit UiEventHandler(Renderer &renderer, const GetMainContextFunc &getMainContextFunc);

 private:
    bool keyPressEvent(const PlatformKeyEvent *event) override;
    bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    bool mouseMoveEvent(const PlatformMouseEvent *event) override;
    bool mousePressEvent(const PlatformMouseEvent *event) override;
    bool mouseReleaseEvent(const PlatformMouseEvent *event) override;
    bool wheelEvent(const PlatformWheelEvent *event) override;
    bool textInputEvent(const PlatformTextInputEvent *event) override;
    bool resizeEvent(const PlatformResizeEvent *event) override;
    bool activationEvent(const PlatformWindowEvent *event) override;

    static int convertMouseButton(PlatformMouseButton button);
    static int getKeyModifierState();
    static Rml::Input::KeyIdentifier convertKey(PlatformKey key);

    GetMainContextFunc _getMainContextFunc;
    Renderer &_renderer;
    Vec2i _mouseOffset;
};
