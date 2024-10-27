#include "Png.h"

#include <memory>
#include <utility>

#define PNG_SIMPLIFIED_READ_SUPPORTED
#define PNG_SIMPLIFIED_WRITE_SUPPORTED
#include <png.h> // NOLINT: not a C system header.

#include "Utility/Exception.h"

RgbaImage png::decode(const Blob &data) {
    png_image pngImage = {};
    pngImage.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&pngImage, data.data(), data.size()))
        throw Exception("Failed to read PNG image '{}' ({}).", data.displayPath(), pngImage.message);

    pngImage.format = PNG_FORMAT_RGBA; // Format we want.

    RgbaImage result = RgbaImage::uninitialized(pngImage.width, pngImage.height);
    if (!png_image_finish_read(&pngImage, nullptr, result.pixels().data(), 0, nullptr)) {
        png_image_free(&pngImage);
        throw Exception("Failed to read PNG image '{}' ({}).", data.displayPath(), pngImage.message);
    }

    png_image_free(&pngImage);
    return result;
}

template<class Color>
static Blob encodeWithFormat(ImageView<Color> image, int format) {
    png_image pngImage = {};
    pngImage.version = PNG_IMAGE_VERSION;
    pngImage.width = image.width();
    pngImage.height = image.height();
    pngImage.format = format;

    size_t size = PNG_IMAGE_PNG_SIZE_MAX(pngImage);
    std::unique_ptr<void, FreeDeleter> data(malloc(size));
    if (!png_image_write_to_memory(&pngImage, data.get(), &size, 0, image.pixels().data(), 0, nullptr))
        throw Exception("Failed to encode PNG image ({})", pngImage.message);

    return Blob::fromMalloc(std::move(data), size);
}

Blob png::encode(RgbaImageView image) {
    return encodeWithFormat(image, PNG_FORMAT_RGBA);
}

Blob png::encode(GrayscaleImageView image) {
    return encodeWithFormat(image, PNG_FORMAT_GRAY);
}
