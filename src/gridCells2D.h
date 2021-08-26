#pragma once
#include <iostream>
#include "utils.h"
#include <math.h>
#include <array>


template<int16_t GS>
class GridCells2D
{
public:
    static constexpr int16_t GRID_SIZE{GS};
    static constexpr int32_t ARR_SIZE{GS*GS};

    constexpr inline static int32_t POS(int32_t i, int32_t j) { return i + GRID_SIZE * j; };
    std::array<XYPair, ARR_SIZE> velocity{};
    std::array<XYPair, ARR_SIZE> velocityCopy{};

    std::array<XYPair, ARR_SIZE> force{};

    std::array<Density, ARR_SIZE> density{};
    std::array<Density, ARR_SIZE> densityCopy{};
};
