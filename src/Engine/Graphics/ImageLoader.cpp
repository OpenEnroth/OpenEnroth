#include "ImageLoader.h"

#include <cassert>
#include <utility>

#include "Engine/Graphics/AtlasLayout.h"
#include "Engine/Resources/ResourceManager.h"

#include "Library/Image/ImageFunctions.h"
#include "Library/Image/Pcx.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/Logger/Logger.h"

RgbaImage Icon_LOD_Loader::loadRgba() {
    RgbaImage result = resources()->icon(name());
    if (!result)
        result = resources()->icon("pending");
    return result;
}

Sizei Icon_LOD_Loader::loadSize() {
    Sizei result = resources()->iconSize(name());
    if (!result)
        result = resources()->iconSize("pending");
    return result;
}

RgbaImage Buff_LOD_Loader::loadRgba() {
    LodImage tex = resources()->rawIcon(name());
    if (!tex.image)
        tex.image = GrayscaleImage::solid(0, 1, 1); // Transparent, the palette is all zeros.

    // So, the way this works.
    //
    // Icons are made using a gradient of 64 different colors. E.g. for a feather (feather fall spell) the
    // gradient literally goes along the length of the feather. The border of the icon is single color.
    //
    // Then, we have a palette that has a colored gradient that's 64 colors long, from lighter to darker colors.
    // We extend this gradient by transposing it and adding it to itself, so we get a gradient 126 colors long.
    // Why 126 colors? Because we don't duplicate the starting & ending colors. The nice thing about the
    // resulting gradient is that it's looped.
    //
    // What we do next can be visualized like this:
    //
    // Color gradient: |-------------------------------------------------------------------------------|
    // Palette:                      |-------------------------------------|
    //
    // We just slide the palette mapping along the gradient, wrapping around as necessary. This results in a nice
    // animation.
    //
    // This used to be done on draw, we're just generating a texture atlas. Alternative is to do this in-shader,
    // but generating an atlas is easier to do.

    AtlasLayout layout({16, 8}, tex.image.size());
    RgbaImage result = RgbaImage::uninitialized(layout.geometry().size());

    for (int i = 0; i < 126; i++) {
        Palette palette;
        palette.colors.fill(Color(0, 0, 0, 0));
        for (int index = 0; index <= 63; index++) {
            int remap = (index + i) % (2 * 63);
            if (remap >= 63)
                remap = (2 * 63) - remap;
            palette.colors[index] = tex.palette.colors[remap];
        }

        Recti cell = layout[i];
        for (int y = 0; y < cell.h; y++)
            for (int x = 0; x < cell.w; x++)
                result[y + cell.y][x + cell.x] = palette.colors[tex.image[y][x]];
    }

    return result;
}

Sizei Buff_LOD_Loader::loadSize() {
    Sizei iconSize = resources()->iconSize(name());
    if (!iconSize)
        iconSize = Sizei(1, 1);
    return AtlasLayout({16, 8}, iconSize).geometry().size();
}

RgbaImage Bitmaps_LOD_Loader::loadRgba() {
    RgbaImage result = resources()->bitmap(name());
    if (!result)
        result = resources()->bitmap("pending");
    return result;
}

Sizei Bitmaps_LOD_Loader::loadSize() {
    Sizei result = resources()->bitmapSize(name());
    if (!result)
        result = resources()->bitmapSize("pending");
    return result;
}

RgbaImage Bitmaps_GEN_Loader::loadRgba() {
    return resources()->generated(name());
}

Sizei Bitmaps_GEN_Loader::loadSize() {
    return resources()->generatedSize();
}

RgbaImage Sprites_LOD_Loader::loadRgba() {
    return resources()->sprite(name());
}

Sizei Sprites_LOD_Loader::loadSize() {
    return resources()->spriteSize(name());
}

static Palette alphaPalette(const Palette &palette) {
    Palette result = palette;
    result.colors[0] = Color();
    return result;
}

static Palette colorKeyPalette(const Palette &palette, Color key) {
    Palette result = palette;
    for (Color &color : result.colors)
        if (color == key)
            color = Color(); // Repeated appearances of the same color do happen.
    return result;
}

RgbaImage LegacyLod_Loader::loadRgba() {
    ResourceManager *res = resources();

    if (_mode == Mode::PCX) {
        Blob data;
        if (_source == Source::ENGLISH_D) {
            if (res->hasEnglishD(name()))
                data = res->englishDData(name());
        } else if (res->hasIcon(name())) {
            data = res->iconData(name());
        }
        if (!data) {
            MM_WARNING("Unable to load {}: entry doesn't exist", name());
            return {};
        }

        RgbaImage result = pcx::decode(data);
        for (Color &pixel : result.pixels())
            if (pixel == _colorKey)
                pixel = Color();
        return result;
    }

    LodImage image;
    switch (_source) {
    case Source::ENGLISH_D:
        image = res->rawEnglishD(name());
        break;
    case Source::ICONS_UI:
        image = res->rawIconsUi(name());
        break;
    case Source::ICONS:
        image = res->rawIcon(res->hasIcon(name()) ? name() : "pending");
        break;
    }
    if (!image.image)
        return {};

    switch (_mode) {
    default:
        assert(false);
        [[fallthrough]];
    case Mode::PALETTED:
        return makeRgbaImage(image.image, image.palette);
    case Mode::COLOR_KEY:
        return makeRgbaImage(image.image, image.zeroIsTransparent ? alphaPalette(image.palette) : colorKeyPalette(image.palette, _colorKey));
    case Mode::SOLID:
        return makeRgbaImage(image.image, image.zeroIsTransparent ? alphaPalette(image.palette) : image.palette);
    case Mode::ALPHA:
        return makeRgbaImage(image.image, alphaPalette(image.palette));
    }
}

Sizei LegacyLod_Loader::loadSize() {
    return loadRgba().size();
}
