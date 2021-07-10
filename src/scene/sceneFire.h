#pragma once
#include "src/scene/sceneBase.h"
#include <cmath>


template<typename GridCellsType>
class SceneFire : public SceneBase
{
public:
    SceneFire(GridCellsType& gc) : mGridCells(gc) {}

    constexpr SceneParams getParams() { return SceneParams{0, // viscosity
                                                           -9, // gravity
                                                           0.99f, // density
                                                           0.001};} // diffusion

    void update([[maybe_unused]] const float time)
    {
        for(int x=0; x<GRID_SIZE; ++x) {
            long rVal = (rand() % 2000)+50;
            mGridCells.density[POS(x, GRID_SIZE-2)].r = rVal / 150.0f;
            mGridCells.density[POS(x, GRID_SIZE-2)].g = sqrt(rVal) / 20.0f;//(rand() % rVal) / 100.0f;

            if (rand() % 50 == 0)  // occasional fire spurt
                mGridCells.velocity[POS(x, GRID_SIZE-5)].y = -rVal / 10.0f;

            if (rand() % (1000000/GRID_SIZE) == 0) // rare downdraft
            {
                for(int i = 1; i < 1000; ++i)
                {
                    mGridCells.force[POS(x, GRID_SIZE * i / 1000.0f)].y += 1e4;
                    int dir = rand() % 3 - 1;
                    mGridCells.force[POS(x, GRID_SIZE * i / 1000.0f)].x += 1e4 * dir;
                }
            }
        }
    }

private:
    GridCellsType& mGridCells;
};
