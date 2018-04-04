#pragma once
#include "constants.hpp"

class GridCells2D
{
public:
  GridCells2D();
  ~GridCells2D();

  float u[SIZE], v[SIZE];
  float u0[SIZE], v0[SIZE];
  float dens[SIZE];
  float temp[SIZE];
};