#pragma once
#include "src/utils.h"
#include <stdint.h>


struct SceneParams
{
    float viscosity{};
    float gravity{};
    float densityTrans{};
    float diffusion{};
};

template<typename GridCellsType>
class SceneBase
{
    auto POS(auto x, auto y) { return GridCellsType::POS(x,y); }
    static constexpr uint16_t GRID_SIZE = GridCellsType::GRID_SIZE;
public:
    SceneBase(GridCellsType& gc) : mGridCells(gc) {}
    virtual ~SceneBase() {};
    virtual void update(const float time) = 0;
    virtual SceneParams getParams() { return SceneParams{0.0001f, // viscosity
                                                           9.81, // gravity
                                                           0.9999f, // density
                                                           0.0f};} // diffusion
protected:
    std::pair<Density, XYPair> getFireSource()
    {
        long rVal = (rand() % 2000)+50;
        return {Density(rVal / 150.0f, sqrt(rVal) / 20.0f, 0),
                XYPair(0, (rand() % 50 == 0) ? -rVal / 10.0f : 0)};
    }

    void addGaussian(const int x, const int y, const int width, const int height, const float r, const float g, const float b, const float u, const float v)
    {
        for(int i=-width/2; i<=width/2; ++i) {
            for(int j=-height/2; j<=height/2; ++j) {
                if (i+x>=0 && j+y>=0 && i+x<GRID_SIZE && j+y<GRID_SIZE) {
                    const  float alpha = -log(0.5)/(GRID_SIZE*GRID_SIZE/2000.0);
                    const float wgt = exp(-alpha * (i*i + j*j));

                    auto satAdd = [](float& x, const float& y) { x = std::min(1.0f, x+y); };

                    satAdd(mGridCells.density[POS(i+x, j+y)].r, r*wgt);
                    satAdd(mGridCells.density[POS(i+x, j+y)].g, g*wgt);
                    satAdd(mGridCells.density[POS(i+x, j+y)].b, b*wgt);
                    mGridCells.velocity[POS(i+x, j+y)] += XYPair{u, v} * wgt;
                }
            }
        }
    }

    GridCellsType& mGridCells;
};
