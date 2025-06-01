#pragma once

#include <cassert>
#include <span>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <utility>

#include "Library/Color/Color.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"
#include "Library/Geometry/Size.h"

#include "Utility/Memory/FreeDeleter.h"
#include "Utility/Types.h"

class Blob;

namespace detail {
template<class T>
class ViewPointer {
 public:
    T *get() const {
        return _ptr;
    }

    void reset(T *ptr) {
        _ptr = ptr;
    }

 private:
    T *_ptr = nullptr;
};

template<class T, class Storage>
class ImageBase {
 public:
    [[nodiscard]] std::span<T> pixels() {
        return {_pixels.get(), _pixels.get() + _width * _height};
    }

    [[nodiscard]] std::span<const T> pixels() const {
        return const_cast<ImageBase &>(*this).pixels();
    }

    [[nodiscard]] ssize_t width() const {
        return _width;
    }

    [[nodiscard]] ssize_t height() const {
        return _height;
    }

    [[nodiscard]] Sizei size() const {
        return Sizei(_width, _height); // Narrowing ssize_t -> int, but we're not expecting images 2B pixels wide.
    }

    [[nodiscard]] Recti rect() const {
        return Recti(Pointi(0, 0), size());
    }

    [[nodiscard]] std::span<T> operator[](ssize_t y) {
        assert(y >= 0 && y < _height);
        return {_pixels.get() + y * _width, _pixels.get() + (y + 1) * _width};
    }

    [[nodiscard]] std::span<const T> operator[](ssize_t y) const {
        return const_cast<ImageBase &>(*this)[y];
    }

    [[nodiscard]] T &operator[](Pointi point) {
        return (*this)[point.y][point.x];
    }

    [[nodiscard]] const T &operator[](Pointi point) const {
        return const_cast<ImageBase &>(*this)[point];
    }

    explicit operator bool() const {
        return static_cast<bool>(_pixels.get());
    }

    void reset() {
        _width = 0;
        _height = 0;
        _pixels.reset();
    }

    void fill(const T &color) {
        std::fill_n(pixels().data(), pixels().size(), color);
    }

 protected: // Directly accessible from derived classes.
    ssize_t _width = 0;
    ssize_t _height = 0;
    Storage _pixels;
};
} // namespace detail


/**
 * `Image` is a class holding a 2d image, with pixels of type `T`.
 *
 * @see ImageView
 */
template<class T>
class Image : public detail::ImageBase<T, std::unique_ptr<T, FreeDeleter>> {
 public:
    Image() = default;
    Image(const Image &) = delete; // Non-copyable.
    Image(Image &&) = default; // Movable.
    Image &operator=(const Image &) = delete; // Non-copyable.
    Image &operator=(Image &&) = default; // Movable.

    Image &operator=(const Image<std::remove_const_t<T>> &&other) requires std::is_const_v<T> {
        using std::swap;
        swap(this->_width, other._width);
        swap(this->_height, other._height);
        swap(this->_pixels, other._pixels);
    }

    /**
     * Constructs an uninitialized image of given size.
     *
     * @param width                     Required image width.
     * @param height                    Required image height.
     * @return                          Uninitialized image of given size. If width or height is zero, then returns an
     *                                  empty image.
     */
    static Image uninitialized(ssize_t width, ssize_t height) {
        assert(width >= 0 && height >= 0);
        if (width == 0 || height == 0)
            return Image();

        Image result;
        result._width = width;
        result._height = height;
        result._pixels.reset(static_cast<T *>(malloc(width * height * sizeof(T))));
        return result;
    }

    /**
     * Creates a solid-filled image of given size.
     *
     * @param width                     Required image width.
     * @param height                    Required image height.
     * @param color                     Color to use to fill the image.
     * @return                          Solid-filled image of given size. If width or height is zero, then returns an
     *                                  empty image.
     */
    static Image solid(ssize_t width, ssize_t height, T color) {
        Image result = uninitialized(width, height);
        std::fill_n(result.pixels().data(), result.pixels().size(), color);
        return result;
    }

    /**
     * Creates an image by copying the provided pixel buffer.
     *
     * @param width                     Pixel buffer width.
     * @param height                    Pixel buffer height.
     * @param pixels                    Pixel buffer to copy.
     * @return                          Newly allocated `Image` containing a copy of the provided pixel buffer.
     */
    static Image copy(ssize_t width, ssize_t height, const T *pixels) {
        Image result = uninitialized(width, height);
        std::copy_n(pixels, result.pixels().size(), result.pixels().data());
        return result;
    }

    /**
     * Creates a copy of another image.
     *
     * @param other                     Image to copy.
     * @return                          Newly allocated `Image` containing a copy of `other`.
     */
    static Image copy(const Image &other) {
        return copy(other.width(), other.height(), other.pixels().data());
    }

    // The rest is inherited from ImageBase.
};


/**
 * `ImageView` to `Image` is what `std::string_view` is to `std::string`.
 */
template<class T>
class ImageView : public detail::ImageBase<const T, detail::ViewPointer<const T>> {
 public:
    ImageView() = default;

    // Default copy & move are OK.

    ImageView(const T *pixels, ssize_t width, ssize_t height) {
        this->_width = width;
        this->_height = height;
        this->_pixels.reset(pixels);
    }

    ImageView(const Image<T> &image) { // NOLINT: intentionally implicit
        this->_width = image.width();
        this->_height = image.height();
        this->_pixels.reset(image.pixels().data());
    }
};

using RgbaImage = Image<Color>;
using RgbaImageView = ImageView<Color>;
using GrayscaleImage = Image<uint8_t>;
using GrayscaleImageView = ImageView<uint8_t>;
