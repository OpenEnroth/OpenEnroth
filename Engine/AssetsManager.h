#pragma once

#include <map>

#include "Engine/Strings.h"

class Image;
class Texture;

class AssetsManager {
 public:
  AssetsManager() {}

  bool ReleaseAllImages();

  bool ReleaseImage(const String & name);

  Image *GetImage_Solid(const String &name);
  Image *GetImage_ColorKey(const String &name, uint16_t colorkey);
  Image *GetImage_Alpha(const String &name);

  Image *GetImage_PCXFromFile(const String &filename);
  Image *GetImage_PCXFromIconsLOD(const String &name);
  Image *GetImage_PCXFromNewLOD(const String &name);

  Texture *GetBitmap(const String &name);
  Texture *GetSprite(const String &name, unsigned int palette_id, unsigned int lod_sprite_id);

 protected:
  std::map<String, Texture*> bitmaps;
  std::map<String, Texture*> sprites;
  std::map<String, Image*> images;
};

extern AssetsManager *assets;
