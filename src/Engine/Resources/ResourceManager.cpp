#include "ResourceManager.h"

#include <algorithm>
#include <cassert>
#include <string>

#include "Engine/Data/ResourceMask.h"
#include "Engine/GameType.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/Json/Json.h"
#include "Library/LodFormats/LodFont.h"
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

ResourceManager::ResourceManager() : _tileGenerator(std::make_unique<TileGenerator>(this)) {}
ResourceManager::~ResourceManager() = default;

void ResourceManager::open(float saturation, float lightness) {
    _saturation = saturation;
    _lightness = lightness;
    // MM8 installs ship no `events.lod`, the same tables and per-map scripts are in `EnglishT.lod` under the same
    // entry names.
    if (dfs->exists("data/events.lod"))
        _eventsLodReader.open(dfs->read("data/events.lod"));
    else if (dfs->exists("data/englisht.lod"))
        _eventsLodReader.open(dfs->read("data/englisht.lod"));
    _iconsLodReader.open(dfs->read("data/icons.lod"));
    _bitmapsLodReader.open(dfs->read("data/bitmaps.lod"));
    _spritesLodReader.open(dfs->read("data/sprites.lod"));
    from_json(Json::parse(dfs->read("data/resource_mask_table.json").str()), _masks);

    // MM8 keeps its localized UI art in EnglishD.lod, which has duplicate entries for some sounds.
    if (gameType() == GAME_TYPE_MM8 && dfs->exists("data/englishd.lod"))
        _englishDLodReader.open(dfs->read("data/englishd.lod"), LOD_ALLOW_DUPLICATES);
    // MM8's viewport border frame and other UI chrome that isn't in icons.lod under any name.
    if (gameType() == GAME_TYPE_MM8 && dfs->exists("data/icons.ui.lod"))
        _iconsUiLodReader.open(dfs->read("data/icons.ui.lod"));
    // TODO(captainurist):
    //  on exception:
    //      Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // but we can't use localization object here cause it's not yet initialized.
}

Blob ResourceManager::eventsData(std::string_view filename) {
    if (!_eventsLodReader.isOpen())
        return Blob();
    // Unlike MM7's events.lod, MM8's EnglishT.lod is missing a handful of table entries MM7
    // always shipped (e.g. `npcdist.txt`) - callers that need this data already tolerate an empty
    // Blob (that's the same shape as "container missing" above), so return one instead of throwing.
    if (gameType() == GAME_TYPE_MM8 && !_eventsLodReader.exists(filename))
        return Blob();
    return lod::decodeMaybeCompressed(_eventsLodReader.read(filename));
}

Blob ResourceManager::eventsText(std::string_view filename) {
    Blob result = eventsData(filename);
    return result.subBlob(0, result.str().find('\x1A')); // The Czech fan translation ends some of its text tables with a 0x1A.
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

LodFont ResourceManager::font(std::string_view filename) {
    return lod::decodeFont(lod::decodeMaybeCompressed(_iconsLodReader.read(filename)));
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

bool ResourceManager::hasIcon(std::string_view filename) const {
    return _iconsLodReader.isOpen() && _iconsLodReader.exists(ascii::toLower(filename));
}

bool ResourceManager::hasEnglishD(std::string_view filename) const {
    return _englishDLodReader.isOpen() && _englishDLodReader.exists(ascii::toLower(filename));
}

bool ResourceManager::hasIconsUi(std::string_view filename) const {
    return _iconsUiLodReader.isOpen() && _iconsUiLodReader.exists(ascii::toLower(filename));
}

Blob ResourceManager::iconData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_iconsLodReader.read(ascii::toLower(filename)));
}

Blob ResourceManager::englishDData(std::string_view filename) {
    return lod::decodeMaybeCompressed(_englishDLodReader.read(ascii::toLower(filename)));
}

LodImage ResourceManager::rawEnglishD(std::string_view filename) {
    if (!hasEnglishD(filename))
        return {};
    return lod::decodeImageMm8(_englishDLodReader.read(ascii::toLower(filename)));
}

LodImage ResourceManager::rawIconsUi(std::string_view filename) {
    if (!hasIconsUi(filename))
        return {};
    return lod::decodeImage(_iconsUiLodReader.read(ascii::toLower(filename)));
}

bool ResourceManager::hasBitmap(std::string_view filename) const {
    return _bitmapsLodReader.isOpen() && _bitmapsLodReader.exists(ascii::toLower(filename));
}

bool ResourceManager::hasSprite(std::string_view filename) const {
    return _spritesLodReader.isOpen() && _spritesLodReader.exists(ascii::toLower(filename));
}
