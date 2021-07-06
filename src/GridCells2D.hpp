#pragma once
#define EIGEN_NO_AUTOMATIC_RESIZING 1
#define EIGEN_STACK_ALLOCATION_LIMIT 0
#include <eigen3/Eigen/Dense>
#include <iostream>


template<int GRID_SIZE>
class GridCells2D
{
public:
  GridCells2D() : u(), v(), u0(), v0(), fx(), fy()
  {
      std::cout << "Grid size: " << GRID_SIZE << std::endl;
  };
  ~GridCells2D() {}

  using MatCellsType = Eigen::Matrix<float, GRID_SIZE, GRID_SIZE>;
  float u[GRID_SIZE * GRID_SIZE], v[GRID_SIZE * GRID_SIZE];
  float u0[GRID_SIZE * GRID_SIZE], v0[GRID_SIZE * GRID_SIZE];
  float fx[GRID_SIZE * GRID_SIZE], fy[GRID_SIZE * GRID_SIZE];

  MatCellsType densityR{MatCellsType::Zero()};
  MatCellsType densityG{MatCellsType::Zero()};
  MatCellsType densityB{MatCellsType::Zero()};

  MatCellsType densityCopyR{MatCellsType::Zero()};
  MatCellsType densityCopyG{MatCellsType::Zero()};
  MatCellsType densityCopyB{MatCellsType::Zero()};
};
