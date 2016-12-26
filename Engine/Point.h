#pragma once

struct Point
{
    inline Point() : x(0), y(0) {}
    inline Point(int _x, int _y) : x(_x), y(_y) {}

    int x;
    int y;
};