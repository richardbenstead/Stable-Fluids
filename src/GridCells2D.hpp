#pragma once

template<int SIZE>
class GridCells2D
{
public:
  GridCells2D() : u(), v(), u0(), v0(), fx(), fy(), dens() {};
  ~GridCells2D() {}

  float u[SIZE], v[SIZE];
  float u0[SIZE], v0[SIZE];
  float fx[SIZE], fy[SIZE];
  float dens[SIZE];
};
