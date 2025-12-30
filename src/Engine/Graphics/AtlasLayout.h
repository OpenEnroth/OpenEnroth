#pragma once

#include <cassert>
#include <cstddef>

#include "Library/Geometry/Rect.h"
#include "Library/Geometry/Size.h"

/**
 * Describes the layout of a texture atlas as a grid of equally-sized cells.
 *
 * This class is purely metadata - it doesn't hold any image data. Use it together with
 * a texture to get source rectangles for individual cells.
 */
class AtlasLayout {
 public:
    AtlasLayout() = default;

    /**
     * @param gridSize              Number of columns and rows in the grid.
     * @param cellSize              Size of each cell in pixels.
     */
    AtlasLayout(Sizei gridSize, Sizei cellSize) : _gridSize(gridSize), _cellSize(cellSize) {
        assert(gridSize.w > 0 && gridSize.h > 0);
        assert(cellSize.w > 0 && cellSize.h > 0);
    }

    /**
     * @param index                 Cell index in row-major order.
     * @return                      Source rectangle for the cell.
     */
    [[nodiscard]] Recti operator[](size_t index) const {
        assert(index < size());
        int column = static_cast<int>(index) % _gridSize.w;
        int row = static_cast<int>(index) / _gridSize.w;
        return {column * _cellSize.w, row * _cellSize.h, _cellSize.w, _cellSize.h};
    }

    /**
     * @return                      Total number of cells in the atlas.
     */
    [[nodiscard]] size_t size() const {
        return static_cast<size_t>(_gridSize.w) * _gridSize.h;
    }

    /**
     * @return                      Total atlas geometry (bounding rectangle starting at origin).
     */
    [[nodiscard]] Recti geometry() const {
        return {0, 0, _gridSize.w * _cellSize.w, _gridSize.h * _cellSize.h};
    }

 private:
    Sizei _gridSize;
    Sizei _cellSize;
};
