#include "Texture_MM7.h"

#include <cstring>
#include <cstdlib>

#include "Engine/ErrorHandling.h"

void Texture_MM7::Release() {
    name.clear();
    indexed = GrayscaleImage();
}
