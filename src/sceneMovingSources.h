#pragma once
#include "constants.h"
#include "utils.h"
#include <vector>
#include <cmath>


template<typename GridCellsType>
class SceneMovingSources
{
public:
    SceneMovingSources(GridCellsType& gc) : mGridCells(gc)
    {}

    constexpr float getViscosity() { return 0.001f; }
    constexpr float getGravity() { return 9.81f; }

    void update(float time)
    {
        constexpr float velWgt = DT * 10.0f;
        constexpr int size = std::max(1, GRID_SIZE/5);

        struct SourceInfo {
            float xAmp, xOffset, xPhase, xSpeed;
            float yAmp, yOffset, yPhase, ySpeed;
            float r,g,b;
        };

        std::vector<SourceInfo> mSources;
        mSources.push_back(SourceInfo{GRID_SIZE * 0.4f, GRID_SIZE/2, 0, 10,
                                      GRID_SIZE * 0.4f, GRID_SIZE/2, 0, 13,
                                      0.1, 0, 0});

        mSources.push_back(SourceInfo{GRID_SIZE * 0.4f, GRID_SIZE/2, 0.5, 12,
                                      GRID_SIZE * 0.4f, GRID_SIZE/2, 0.5, 15,
                                      0.1, 0.1, 0});

        mSources.push_back(SourceInfo{GRID_SIZE * 0.4f, GRID_SIZE/2, 1, 7,
                                      GRID_SIZE * 0.4f, GRID_SIZE/2, 1, 5,
                                      0, 0.1, 0});

        mSources.push_back(SourceInfo{GRID_SIZE * 0.4f, GRID_SIZE/2, 2, 11,
                                      GRID_SIZE * 0.4f, GRID_SIZE/2, 2, 8,
                                      0, 0, 0.1});

        for(auto s : mSources) {
            addBlock(sin(time * s.xSpeed + s.xPhase) * s.xAmp + s.xOffset,
                     sin(time * s.ySpeed + s.yPhase) * s.yAmp + s.yOffset,
                     size, size, s.r, s.g, s.b,
                     velWgt * cos(time * s.xSpeed + s.xPhase) * s.xAmp,
                     velWgt * cos(time * s.ySpeed + s.yPhase) * s.yAmp);
        }
    }

    void addBlock(const int x, const int y, const int width, const int height, const float r, const float g, const float b, const float u, const float v)
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

private:
    GridCellsType& mGridCells;
};
