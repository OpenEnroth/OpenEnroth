#include "ResourceManager.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>

#include "Engine/Data/ResourceMask.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/Json/Json.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/LodFormats/LodSprite.h"
#include "Library/Logger/Logger.h"
#include "Library/FileSystem/Interface/FileSystem.h"

#include "Utility/Math/Float.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Format.h"
#include "Utility/Lambda.h"
#include "Utility/MapAccess.h"

#include "EngineFileSystem.h"
#include "TileGenerator.h"

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

// TODO(captainurist): move the edge blending and the PCX color keying into Library/Image. Sprites and generated
//                     tiles will need them too.
static Color processTransparentPixel(const GrayscaleImage &image, const Palette &palette, size_t x, size_t y) {
    size_t count = 0;
    size_t r = 0, g = 0, b = 0;

    auto processPixel = [&](size_t x, size_t y) {
        uint8_t pal = image[y][x];
        if (palette.colors[pal].a != 0) {
            count++;
            r += palette.colors[pal].r;
            g += palette.colors[pal].g;
            b += palette.colors[pal].b;
        }
    };

    bool canDecX = x > 0;
    bool canIncX = x < image.width() - 1;
    bool canDecY = y > 0;
    bool canIncY = y < image.height() - 1;

    if (canDecX && canDecY)
        processPixel(x - 1, y - 1);
    if (canDecX)
        processPixel(x - 1, y);
    if (canDecX && canIncY)
        processPixel(x - 1, y + 1);
    if (canDecY)
        processPixel(x, y - 1);
    if (canIncY)
        processPixel(x, y + 1);
    if (canIncX && canDecY)
        processPixel(x + 1, y - 1);
    if (canIncX)
        processPixel(x + 1, y);
    if (canIncX && canIncY)
        processPixel(x + 1, y + 1);

    if (count != 0) {
        r /= count;
        g /= count;
        b /= count;
    }

    return Color(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), 0);
}

static Palette grayscalePalette() {
    Palette result;
    for (int i = 0; i < 256; i++)
        result.colors[i] = Color(i, i, i, 255);
    return result;
}

namespace {

struct EncodingMarker {
    TextEncoding encoding;
    std::string_view word; // Word for "sword", spelled & encoded the way `encoding`'s localization spells it.
};

/**
 * Every localization ships the same `items.txt`, and every one of them has swords in it. So the word for "sword",
 * in the localization's own encoding, identifies the encoding.
 *
 * Charset detection doesn't work here. English game data is 99.98% ASCII, and the handful of non-ASCII bytes it
 * does have are punctuation that windows-1251 and windows-1252 map the same way - there's simply nothing to detect.
 *
 * Note that the English word can't be used as a marker: the localized files keep an English "Skill Group" column,
 * so "sword" is in all of them. English is what we fall back to instead.
 */
constexpr EncodingMarker encodingMarkers[] = {
    {ENCODING_WINDOWS_1251, "\xEC\xE5\xF7"},        // "меч", Russian.
    {ENCODING_WINDOWS_1251, "\xCC\xE5\xF7"},        // "Меч".
    {ENCODING_WINDOWS_1252, "\xE9p\xE9\x65"},       // "épée", French.
    {ENCODING_WINDOWS_1252, "\xC9p\xE9\x65"},       // "Épée".
    {ENCODING_WINDOWS_1252, "schwert"},             // "schwert", German.
    {ENCODING_WINDOWS_1252, "Schwert"},
};

} // namespace

ResourceManager::ResourceManager() : _tileGenerator(std::make_unique<TileGenerator>(this)) {}
ResourceManager::~ResourceManager() = default;

void ResourceManager::open(float saturation, float lightness) {
    _saturation = saturation;
    _lightness = lightness;
    _eventsLodReader.open(dfs->read("data/events.lod"));
    _iconsLodReader.open(dfs->read("data/icons.lod"));
    _bitmapsLodReader.open(dfs->read("data/bitmaps.lod"));
    _spritesLodReader.open(dfs->read("data/sprites.lod"));
    from_json(Json::parse(dfs->read("data/resource_mask_table.json").str()), _masks);
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    //  but we can't use localization object here cause it's not yet initialized.

    Blob items = eventsData("items.txt");
    std::string_view itemsText = items.str();

    _gameDataEncoding = ENCODING_WINDOWS_1252;
    for (const EncodingMarker &marker : encodingMarkers) {
        if (itemsText.contains(marker.word)) {
            _gameDataEncoding = marker.encoding;
            break;
        }
    }
}

Blob ResourceManager::eventsData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

RgbaImage ResourceManager::icon(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_iconsLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _iconsLodReader.displayPath(name));
        return {};
    }

    ResourceMask mask = valueOr(_masks.icons, name);
    if (name.ends_with(".pcx")) {
        RgbaImage result = pcx::decode(lod::decodeMaybeCompressed(_iconsLodReader.read(name)));
        if (mask.mode == MASK_COLOR)
            for (Color &pixel : result.pixels())
                if (pixel == mask.color)
                    pixel = Color();
        return result;
    }

    LodImage image = lod::decodeImage(_iconsLodReader.read(name));
    return makeRgbaImage(image.image, maskedPalette(image, mask));
}

Sizei ResourceManager::iconSize(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_iconsLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _iconsLodReader.displayPath(name));
        return {};
    }

    if (name.ends_with(".pcx"))
        return pcx::decodeSize(lod::decodeMaybeCompressed(_iconsLodReader.read(name)));
    return lod::decodeImageSize(_iconsLodReader.read(name));
}

RgbaImage ResourceManager::bitmap(std::string_view filename) {
    LodImage image = rawBitmap(filename);
    if (!image.image)
        return {};

    std::string name = ascii::toLower(filename);
    Palette palette = maskedPalette(image, valueOr(_masks.bitmaps, name));
    desaturate(palette.colors);
    if (std::ranges::all_of(palette.colors, _1 != 0, &Color::a))
        return makeRgbaImage(image.image, palette);

    // Bitmaps are drawn with bilinear filtering, so transparent pixels take the color of their opaque neighbors
    // to keep the filter from bleeding the mask color into the edges.
    RgbaImage result = RgbaImage::uninitialized(image.image.width(), image.image.height());
    for (size_t y = 0; y < image.image.height(); y++) {
        for (size_t x = 0; x < image.image.width(); x++) {
            uint8_t pal = image.image[y][x];
            if (palette.colors[pal].a == 0) {
                result[y][x] = processTransparentPixel(image.image, palette, x, y);
            } else {
                result[y][x] = palette.colors[pal];
            }
        }
    }
    return result;
}

Sizei ResourceManager::bitmapSize(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_bitmapsLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _bitmapsLodReader.displayPath(name));
        return {};
    }
    return lod::decodeImageSize(_bitmapsLodReader.read(name));
}

RgbaImage ResourceManager::generated(std::string_view filename) {
    _tileGenerator->ensureTile(filename);
    RgbaImage result = png::decode(ufs->read(filename));
    desaturate(result.pixels());
    return result;
}

Sizei ResourceManager::generatedSize() const {
    return _tileGenerator->tileSize();
}

void ResourceManager::addGeneratedTiles(TileTable *table) {
    _tileGenerator->fillTable(table);
}

LodImage ResourceManager::rawIcon(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_iconsLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _iconsLodReader.displayPath(name));
        return {};
    }
    return lod::decodeImage(_iconsLodReader.read(name));
}

LodImage ResourceManager::rawBitmap(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_bitmapsLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _bitmapsLodReader.displayPath(name));
        return {};
    }
    return lod::decodeImage(_bitmapsLodReader.read(name));
}

Palette ResourceManager::palette(int paletteId) {
    std::string name = fmt::format("pal{:03}", paletteId);
    if (!_bitmapsLodReader.exists(name))
        return grayscalePalette();

    Palette result = lod::decodeImage(_bitmapsLodReader.read(name)).palette;
    desaturate(result.colors);
    return result;
}

RgbaImage ResourceManager::sprite(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_spritesLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _spritesLodReader.displayPath(name));
        return {};
    }

    LodSprite sprite = lod::decodeSprite(_spritesLodReader.read(name));
    RgbaImage result = RgbaImage::uninitialized(sprite.image.width(), sprite.image.height());
    auto srcPixels = sprite.image.pixels();
    auto dstPixels = result.pixels();
    for (size_t i = 0, size = srcPixels.size(); i < size; i++)
        dstPixels[i] = Color(srcPixels[i], 0, 0, srcPixels[i] == 0 ? 0 : 255);
    return result;
}

Sizei ResourceManager::spriteSize(std::string_view filename) {
    std::string name = ascii::toLower(filename);
    if (!_spritesLodReader.exists(name)) {
        MM_ERROR("Trying to load non-existent LOD entry '{}'.", _spritesLodReader.displayPath(name));
        return {};
    }
    return lod::decodeSpriteSize(_spritesLodReader.read(name));
}

void ResourceManager::desaturate(std::span<Color> colors) const {
    if (fuzzyEquals(_saturation, 1.0f) && fuzzyEquals(_lightness, 1.0f))
        return;

    for (Color &color : colors)
        color = color.toHsvColorf().adjusted(0, _saturation, _lightness).toColor();
}

std::string ResourceManager::eventsText(std::string_view filename) {
    return txt::encodedToUtf8(eventsData(filename).str(), _gameDataEncoding);
}
