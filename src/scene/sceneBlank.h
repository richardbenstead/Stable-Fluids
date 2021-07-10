#pragma once
#include "src/scene/sceneBase.h"


template<typename GridCellsType>
class SceneBlank : public SceneBase
{
public:
    SceneBlank(GridCellsType& gc) : mGridCells(gc) {}

    constexpr SceneParams getParams() { return SceneParams{0, // viscosity
                                                           20, // gravity
                                                           1.0f, // density
                                                           0.01f};} // diffusion

    void update([[maybe_unused]] const float time)
    {
        for(int j=0; j<GRID_SIZE/10; ++j) {
            for(int i=0; i<GRID_SIZE/5; ++i) {
                const int idx = POS(1+j, GRID_SIZE/5 + i);
                mGridCells.density[idx] = {0.5, 0.5, 0.8};
                mGridCells.force[idx].x = 100;
            }
        }
    }

private:
    GridCellsType& mGridCells;
};

