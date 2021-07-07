#pragma once
#include "sceneBase.h"
#include "constants.h"
#include "utils.h"
#include <cmath>


template<typename GridCellsType>
class SceneFire : public SceneBase
{
public:
    SceneFire(GridCellsType& gc) : mGridCells(gc) {}

    constexpr float getViscosity() { return 0.00001f; }
    constexpr float getGravity() { return -10.0f; }
    constexpr float getPressureTrans() { return 0.99f; }

    void update([[maybe_unused]] const float time)
    {
        for(int x=0; x<GRID_SIZE; ++x) {
            long rVal = (rand() % 100)+50;
            mGridCells.density[POS(x, GRID_SIZE-1)].r += rVal / 150.0f;
            mGridCells.density[POS(x, GRID_SIZE-1)].g += sqrt(rVal) / 40.0f;//(rand() % rVal) / 100.0f;

            if (rand() % 10 == 0)
            {
                int rVal = rand() % 100;
                int dir = (rand() % 3) - 1;
                float xVel = (rVal * rVal * dir) / 100.0f;
                mGridCells.velocity[POS(x, GRID_SIZE-2)].x += xVel;
                mGridCells.velocity[POS(x, GRID_SIZE-2)].y += -(rand() % 100) / 1.0f;
            }
        }
    }

private:
    GridCellsType& mGridCells;
};
