#pragma once

template<class T>
struct Margins {
    T left = 0;
    T top = 0;
    T right = 0;
    T bottom = 0;

    Margins() = default;
    Margins(T left, T top, T right, T bottom): left(left), top(top), right(right), bottom(bottom) {}
};

using Marginsi = Margins<int>;
