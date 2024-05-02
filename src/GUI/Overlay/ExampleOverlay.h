#pragma once

#include "Overlay.h"

class ExampleOverlay : public Overlay {
 public:
    virtual void update(nk_context *context) override;
};
