#include "ImageLoader.h"

#include <string_view>
#include <memory>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Graphics/AtlasLayout.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/TileGenerator.h"
#include "Engine/Resources/ResourceManager.h"

#include "Library/Image/Pcx.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodImage.h"
#include "Library/Logger/Logger.h"

#include "Utility/Math/Float.h"

bool Icon_LOD_Loader::Load(RgbaImage *rgbaImage) {
    *rgbaImage = resources->icon(resource_name);
    if (!*rgbaImage)
        *rgbaImage = resources->icon("pending");
    return true;
}

bool Buff_LOD_Loader::Load(RgbaImage *rgbaImage) {
    LodImage tex = resources->rawIcon(resource_name);
    if (!tex.image) {
        *rgbaImage = RgbaImage::solid(Color(), 16, 8); // One transparent pixel per atlas cell.
        return true;
    }

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

    *rgbaImage = std::move(result);

    return true;
}

bool PCX_Loader::InternalLoad(const Blob &data, RgbaImage *rgbaImage) {
    *rgbaImage = pcx::decode(data);
    return true;
}

bool PCX_LOD_Raw_Loader::Load(RgbaImage *rgbaImage) {
    Blob data = lod->read(resource_name);
    if (!data) {
        MM_WARNING("Unable to load {}", this->resource_name);
        return false;
    }

    return InternalLoad(data, rgbaImage);
}

bool Bitmaps_LOD_Loader::Load(RgbaImage *rgbaImage) {
    *rgbaImage = resources->bitmap(resource_name);
    if (!*rgbaImage)
        *rgbaImage = resources->bitmap("pending");
    return true;
}

bool Bitmaps_GEN_Loader::Load(RgbaImage *rgbaImage) {
    pTileGenerator->ensureTile(this->resource_name);
    *rgbaImage = png::decode(ufs->read(this->resource_name));

    // Desaturate.
    float xs = engine->config->graphics.Saturation.value();
    float xv = engine->config->graphics.Lightness.value();
    if (fuzzyEquals(xs, 1.0f) && fuzzyEquals(xv, 1.0f))
        return true;

    for (Color &pixel : rgbaImage->pixels())
        pixel = pixel.toHsvColorf().adjusted(0, xs, xv).toColor();

    return true;
}

bool Sprites_LOD_Loader::Load(RgbaImage *rgbaImage) {
    *rgbaImage = resources->sprite(resource_name);
    return static_cast<bool>(*rgbaImage);
}

