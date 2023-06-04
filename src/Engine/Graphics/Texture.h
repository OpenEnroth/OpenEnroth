#pragma once

#include "Engine/Graphics/Image.h"

class Texture : public GraphicsImage {
 protected:
    explicit Texture(bool lazy_initialization = true)
        : GraphicsImage(lazy_initialization) {}
};
