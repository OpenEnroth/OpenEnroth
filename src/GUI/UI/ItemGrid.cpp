#include "ItemGrid.h"

#include <cassert>

static inline int roundIntDownToBase(int i, int base) {
    return i - i % base;
}

int itemOffset(int widthOrHeight) {
    int width = widthOrHeight;
    if (width < 14) width = 14;

    int X_offset = (roundIntDownToBase(width - 14, 32) + 32 - width) / 2;

    assert(-6 <= X_offset && X_offset <= 9);
    return X_offset;
}
