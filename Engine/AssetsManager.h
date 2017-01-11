#pragma once
#include <map>

#include "Engine/Strings.h"

class Image;
class Texture;
class AssetsManager
{
    public:
        AssetsManager() {}

        bool ReleaseAllImages();

        Image *GetImage_16Bit(const String &name);
        Image *GetImage_16BitColorKey(const String &name, unsigned __int16 colorkey);
        Image *GetImage_16BitAlpha(const String &name);

        Image *GetImage_PCXFromFile(const String &filename);
        Image *GetImage_PCXFromIconsLOD(const String &name);
        Image *GetImage_PCXFromNewLOD(const String &name);

        Texture *GetBitmap(const String &name);

    protected:
		std::map<String, Texture *> bitmaps;
};

extern AssetsManager *assets;