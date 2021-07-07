#pragma once
#include <cmath>

constexpr inline int POS(int i, int j) { return i + GRID_SIZE * j; };

struct XYPair
{
    float x{}, y{};
    XYPair operator*(const float f) const { return XYPair{x*f, y*f}; }
    XYPair operator-(const XYPair& xy) const { return XYPair{x-xy.x, y-xy.y}; }
    XYPair operator+(const XYPair& xy) const { return XYPair{x+xy.x, y+xy.y}; }
    XYPair operator+=(const XYPair& xy) {
        x += xy.x;
        y += xy.y;
        return *this;
    }
    float norm() const { return sqrt(x*x+y*y); }
};

struct Density
{
    float r{}, g{}, b{};
    Density operator*(const float f) const { return Density{r*f, g*f, b*f}; }
    Density operator+(const Density& den) const { return Density{r+den.r, g+den.g, b+den.b}; }
    Density operator+=(const Density& den) {
        r += den.r;
        g += den.g;
        b += den.b;
        return *this;
    }
};

