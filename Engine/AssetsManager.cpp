#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/LOD.h"

AssetsManager *assets = new AssetsManager();


Image *AssetsManager::GetImage_16BitColorKey(const String &name, unsigned __int16 colorkey)
{
    return this->GetImage_16BitColorKey(name.c_str(), colorkey);
}

Image *AssetsManager::GetImage_16BitColorKey(const char *name, unsigned __int16 colorkey)
{
    wchar_t namew[1024];
    swprintf(namew, L"%S", name);

    return this->GetImage_16BitColorKey(namew, colorkey);
}

Image *AssetsManager::GetImage_16BitColorKey(const wchar_t *name, unsigned __int16 colorkey)
{
    Image *img = new Image();
    if (!img->ColorKey_From_LOD(name, colorkey))
    {
        delete img;
        img = nullptr;
    }

    return img;
}






Image *AssetsManager::GetImage_16Bit(const String &name)
{
    return this->GetImage_16Bit(name.c_str());
}

Image *AssetsManager::GetImage_16Bit(const char *name)
{
    wchar_t wname[1024];
    swprintf(wname, L"%S", name);

    return this->GetImage_16Bit(wname);
}

Image *AssetsManager::GetImage_16Bit(const wchar_t *name)
{
    Image *img = new Image();
    if (!img->Image16bit_From_LOD(name))
    {
        delete img;
        img = nullptr;
    }

    return img;
}









Image *AssetsManager::GetImage_16BitAlpha(const String &name)
{
    return this->GetImage_16BitAlpha(name.c_str());

}

Image *AssetsManager::GetImage_16BitAlpha(const char *name)
{
    wchar_t wname[1024];
    swprintf(wname, L"%S", name);

    return this->GetImage_16BitAlpha(wname);
}

Image *AssetsManager::GetImage_16BitAlpha(const wchar_t *name)
{
    Image *img = new Image();
    if (!img->Alpha_From_LOD(name))
    {
        delete img;
        img = nullptr;
    }

    return img;
}





Image *AssetsManager::GetImage_PCXFromIconsLOD(const wchar_t *name)
{
    Image *img = new Image();
    if (!img->PCX_From_IconsLOD(name))
    {
        delete img;
        img = nullptr;
    }

    return img;
}

Image *AssetsManager::GetImage_PCXFromNewLOD(const wchar_t *name)
{
    Image *img = new Image();
    if (!img->PCX_From_NewLOD(name))
    {
        delete img;
        img = nullptr;
    }

    return img;
}


Image *AssetsManager::GetImage_PCXFromFile(const wchar_t *name)
{
    Image *img = new Image();
    if (!img->PCX_From_File(name))
    {
        delete img;
        img = nullptr;
    }

    return img;
}