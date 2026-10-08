#include "ImageLoader.h"

#include "Engine/Graphics/AtlasLayout.h"
#include "Engine/Resources/ResourceManager.h"

#include "Library/LodFormats/LodImage.h"

RgbaImage Icon_LOD_Loader::load() {
    RgbaImage result = resources()->icon(name());
    if (!result)
        result = resources()->icon("pending");
    return result;
}

RgbaImage Buff_LOD_Loader::load() {
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

RgbaImage Bitmaps_LOD_Loader::load() {
    RgbaImage result = resources()->bitmap(name());
    if (!result)
        result = resources()->bitmap("pending");
    return result;
}

RgbaImage Bitmaps_GEN_Loader::load() {
    return resources()->generated(name());
}

RgbaImage Sprites_LOD_Loader::load() {
    return resources()->sprite(name());
}

