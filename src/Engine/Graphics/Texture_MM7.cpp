#include "Texture_MM7.h"

void Texture_MM7::Release() {
    name.clear();
    indexed = GrayscaleImage();
}
