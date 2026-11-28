#include "FontFiles.h"

#include <string>

#include "Library/Font/FontSheet.h"
#include "Library/Font/Oef.h"
#include "Library/Image/Png.h"
#include "Library/LodFormats/LodFormats.h"
#include "Library/Magic/Magic.h"

#include "Utility/Exception.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/Streams/FileOutputStream.h"

#include "Bin/LodTool/ArchiveReader.h"

Font readFont(PathView path, TextEncoding encoding) {
    Blob data = lod::decodeMaybeCompressed(ArchiveReader::readFile(path));
    switch (magic(data)) {
    case MAGIC_LOD_FONT:
        return lod::decodeFont(data, encoding);
    case MAGIC_OE_FONT:
        return oef::decode(data);
    default:
        throw Exception("'{}' is not a font", path.displayString());
    }
}

void writeFont(PathView path, const Font &font) {
    std::string extension = ascii::toLower(path.extension());
    if (extension != ".png" && extension != ".oef")
        throw Exception("Can't write a font to '{}', only .oef and .png are supported", path.displayString());

    Blob data = extension == ".png" ? png::encode(renderFontSheet(font)) : oef::encode(font);
    FileOutputStream output(path);
    output.write(data);
    output.close();
}

uint8_t inkColorIndex(const Font &font) {
    for (int i = 0; i < font.size(); i++)
        for (uint8_t pixel : font.image(i).pixels())
            if (pixel == 1)
                return 1;
    return 2;
}
