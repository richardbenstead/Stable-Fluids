#pragma once
#include <iostream>
#include <array>
#include <math.h>


template<int GRID_SIZE>
class GridCells2D
{
public:
    static constexpr int ARR_SIZE = GRID_SIZE * GRID_SIZE;

    struct XYPair {
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

    struct Density {
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

    std::array<XYPair, ARR_SIZE> velocity{};
    std::array<XYPair, ARR_SIZE> velocityCopy{};

    std::array<XYPair, ARR_SIZE> force{};

    std::array<Density, ARR_SIZE> density{};
    std::array<Density, ARR_SIZE> densityCopy{};
};
