#pragma once
#include "Engine/Graphics/Image.h"

class Texture : public Image {
 protected:
    explicit Texture(bool lazy_initialization = true)
        : Image(lazy_initialization) {}
};
