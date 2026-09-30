#include "ResourceManager.h"

#include <cassert>
#include <string>

#include "Engine/Data/ResourceMask.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Json/Json.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"

#include "EngineFileSystem.h"

/**
 * @param image                     Image to mask.
 * @param mask                      Mask to apply.
 * @return                          The image's palette, with the entries the mask picks made transparent.
 */
static Palette maskedPalette(const LodImage &image, const ResourceMask &mask) {
    Palette result = image.palette;
    switch (mask.mode) {
    default:
        assert(false);
        [[fallthrough]];
    case MASK_DEFAULT:
        if (image.zeroIsTransparent)
            result.colors[0] = Color();
        break;
    case MASK_NONE:
        break;
    case MASK_ZERO:
        result.colors[0] = Color();
        break;
    case MASK_COLOR:
        for (Color &color : result.colors)
            if (color == mask.color)
                color = Color();
        break;
    }
    return result;
}

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::open() {
    _eventsLodReader.open(dfs->read("data/events.lod"));
    _iconsLodReader.open(dfs->read("data/icons.lod"));
    _bitmapsLodReader.open(dfs->read("data/bitmaps.lod"));
    from_json(Json::parse(dfs->read("data/resource_mask_table.json").str()), _masks);
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob ResourceManager::eventsData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

RgbaImage ResourceManager::icon(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    ResourceMask mask = valueOr(_masks.icons, name);

    if (name.ends_with(".pcx")) {
        if (!_iconsLodReader.exists(name))
            return {};

        RgbaImage result = pcx::decode(lod::decodeMaybeCompressed(_iconsLodReader.read(name)));
        if (mask.mode == MASK_COLOR)
            for (Color &pixel : result.pixels())
                if (pixel == mask.color)
                    pixel = Color();
        return result;
    }

    LodImage image = lod::decodeImage(_iconsLodReader.read(_iconsLodReader.exists(name) ? name : "pending"));
    return makeRgbaImage(image.image, maskedPalette(image, mask));
}

LodImage ResourceManager::bitmap(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    LodImage result = lod::decodeImage(_bitmapsLodReader.read(_bitmapsLodReader.exists(name) ? name : "pending"));
    result.palette = maskedPalette(result, valueOr(_masks.bitmaps, name));
    return result;
}
