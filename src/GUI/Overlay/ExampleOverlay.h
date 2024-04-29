#pragma once

#include "Overlay.h"

class ExampleOverlay : public Overlay {
 public:
    void update(nk_context *context) override;
};
