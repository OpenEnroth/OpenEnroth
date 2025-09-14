#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "Engine/Graphics/Image.h"

/**
 * A canvas-like container that allows adding images with associated values, then querying pixels to get the value of
 * the topmost non-transparent image at that position.
 * 
 * @tparam T                            The type of values associated with images.
 */
template<class T>
class HitMap {
 public:
    /**
     * Add an image with associated value to the canvas.
     *
     * @param position                  Top-left position where the image should be placed.
     * @param image                     Pointer to the graphics image.
     * @param value                     Value to associate with this image.
     */
    void add(Pointi position, GraphicsImage *image, T value) {
        assert(image);

        _entries.emplace_back(position, image, std::move(value));
        
        Recti imageRect = Recti(position, image->size());
        if (_entries.size() == 1) {
            _bounds = imageRect;
        } else {
            _bounds |= imageRect;
        }
    }
    
    /**
     * Clear all images from the canvas.
     */
    void clear() {
        _entries.clear();
        _bounds = Recti();
    }
    
    /**
     * Query a pixel position to get the value from the topmost non-transparent image.
     *
     * @param point                     Position to query.
     * @param defaultValue              Value to return if no non-transparent image is found at the position.
     * @return                          Value associated with the topmost non-transparent image at the given position,
     *                                  or `defaultValue` if no such image exists.
     */
    T query(Pointi point, T defaultValue = {}) const {
        if (!_bounds.contains(point))
            return defaultValue;

        // Iterate entries in reverse order (topmost first).
        for (auto it = _entries.rbegin(); it != _entries.rend(); ++it) {
            const Entry &entry = *it;
            
            Pointi relativePos = point - entry.position;
            if (entry.image->rgba().rect().contains(relativePos)) {
                Color color = entry.image->rgba()[relativePos];
                if (color.a > 0)
                    return entry.value;
            }
        }
        
        return defaultValue;
    }

 private:
    struct Entry {
        Pointi position;
        GraphicsImage *image;
        T value;

        Entry(Pointi pos, GraphicsImage *img, T val) : position(pos), image(img), value(val) {}
    };

    std::vector<Entry> _entries;
    Recti _bounds; // Bounding rectangle of all images.
};
