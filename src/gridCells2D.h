#pragma once
#include <iostream>
#include "utils.h"
#include <math.h>
#include <array>

template<int GRID_SIZE>
class GridCells2D
{
public:
    static constexpr int ARR_SIZE = GRID_SIZE * GRID_SIZE;

    std::array<XYPair, ARR_SIZE> velocity{};
    std::array<XYPair, ARR_SIZE> velocityCopy{};

    std::array<XYPair, ARR_SIZE> force{};

    std::array<Density, ARR_SIZE> density{};
    std::array<Density, ARR_SIZE> densityCopy{};
};
