#pragma once


class Image;
class AssetsManager
{
    public:
        AssetsManager() {}

        Image *GetImage_PCXFromFile(const wchar_t *filename);
        Image *GetImage_PCXFromIconsLOD(const wchar_t *name);
        Image *GetImage_PCXFromNewLOD(const wchar_t *name);

        Image *GetImage_16Bit(const wchar_t *name);
        Image *GetImage_16Bit(const char *name);
        Image *GetImage_16BitColorKey(const wchar_t *name, unsigned __int16 colorkey);
        Image *GetImage_16BitColorKey(const char *name, unsigned __int16 colorkey);
        Image *GetImage_16BitAlpha(const wchar_t *name);
        Image *GetImage_16BitAlpha(const char *name);

    protected:
};

extern AssetsManager *assets;