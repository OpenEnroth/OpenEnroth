#pragma once
#include "Engine/Strings.h"

class Image;
class AssetsManager
{
    public:
        AssetsManager() {}

        bool ReleaseAllImages();

        Image *GetImage_PCXFromFile(const String &filename);
        Image *GetImage_PCXFromIconsLOD(const String &name);
        Image *GetImage_PCXFromNewLOD(const String &name);

        //Image *GetImage_16Bit(const wchar_t *name);
        //Image *GetImage_16Bit(const char *name);
        Image *GetImage_16Bit(const String &name);

        //Image *GetImage_16BitColorKey(const wchar_t *name, unsigned __int16 colorkey);
        //Image *GetImage_16BitColorKey(const char *name, unsigned __int16 colorkey);
        Image *GetImage_16BitColorKey(const String &name, unsigned __int16 colorkey);

        //Image *GetImage_16BitAlpha(const wchar_t *name);
        //Image *GetImage_16BitAlpha(const char *name);
        Image *GetImage_16BitAlpha(const String &name);

        Image *GetBitmap(const String &name);

    protected:
};

extern AssetsManager *assets;