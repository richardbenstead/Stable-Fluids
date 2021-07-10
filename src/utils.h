#pragma once
#include <cmath>
#include <iostream>
#include "constants.h"

constexpr inline int POS(int i, int j) { return i + GRID_SIZE * j; };

struct XYPair
{
    XYPair operator*(const float f) const { return XYPair{x*f, y*f}; }
    XYPair operator-(const XYPair& xy) const { return XYPair{x-xy.x, y-xy.y}; }
    XYPair operator+(const XYPair& xy) const { return XYPair{x+xy.x, y+xy.y}; }
    XYPair operator+=(const XYPair& xy) {
        x += xy.x;
        y += xy.y;
        return *this;
    }
    float norm() const { return sqrt(x*x+y*y); }

    float x{}, y{};
};

struct Density
{
    Density operator*(const float f) const { return Density{r*f, g*f, b*f}; }
    Density operator+(const Density& den) const { return Density{r+den.r, g+den.g, b+den.b}; }
    Density operator+=(const Density& den) {
        r += den.r;
        g += den.g;
        b += den.b;
        return *this;
    }
    Density operator=(const Density& den) {
        r = std::max(0.0f, std::min(1.0f, den.r));
        g = std::max(0.0f, std::min(1.0f, den.g));
        b = std::max(0.0f, std::min(1.0f, den.b));
        return *this;
    }

    float r{}, g{}, b{};
};

