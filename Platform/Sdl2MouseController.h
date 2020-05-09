#pragma once

#include "Io/IMouseController.h"

class Sdl2MouseController : public Io::IMouseController {
 public:
    int GetCursorX() const override;
    int GetCursorY() const override;
};
