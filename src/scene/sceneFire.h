#pragma once
#include "src/scene/sceneBase.h"
#include "src/utils.h"
#include <cmath>


template<typename GridCellsType>
class SceneFire : public SceneBase<GridCellsType>
{
    using baseType = SceneBase<GridCellsType>;
    auto POS(auto x, auto y) { return GridCellsType::POS(x,y); }
    static constexpr uint16_t GRID_SIZE = GridCellsType::GRID_SIZE;
public:
    SceneFire(GridCellsType& gc) : baseType(gc) {}

    constexpr SceneParams getParams() { return SceneParams{0, // viscosity
                                                           -9, // gravity
                                                           0.99f, // density
                                                           0.001};} // diffusion

    void update([[maybe_unused]] const float time)
    {
        for(int x=0; x<GRID_SIZE; ++x) {
            auto [den, vel] = baseType::getFireSource();
            baseType::mGridCells.density[POS(x, GRID_SIZE-2)] = den;
            baseType::mGridCells.velocity[POS(x, GRID_SIZE-5)] = vel;

            if (rand() % (1000000/GRID_SIZE) == 0) // rare downdraft
            {
                for(int i = 50; i < 99; ++i)
                {
                    baseType::mGridCells.force[POS(x, GRID_SIZE * i / 1000.0f)].y += 1e4;
                    baseType::mGridCells.force[POS(x, GRID_SIZE * i / 1000.0f)].x += 1e4 * (rand() % 3 - 1);
                }
            }
        }

        constexpr float velWgt = 0.01f;
        constexpr int size = std::max(1, GRID_SIZE/5);
        struct SourceInfo {
            float xAmp, xOffset, xPhase, xSpeed;
            float yAmp, yOffset, yPhase, ySpeed;
            float r,g,b;
        };

        std::vector<SourceInfo> mSources;
        mSources.push_back(SourceInfo{GRID_SIZE * 0.4f, GRID_SIZE/2, 0, 10,
                                      GRID_SIZE * 0.4f, GRID_SIZE/2, 0, 13,
                                      0, 0, 0.05});

        for(auto s : mSources) {
            baseType::addGaussian(sin(time * s.xSpeed + s.xPhase) * s.xAmp + s.xOffset,
                                  sin(time * s.ySpeed + s.yPhase) * s.yAmp + s.yOffset,
                                  size, size, s.r, s.g, s.b,
                                  velWgt * cos(time * s.xSpeed + s.xPhase) * s.xAmp,
                                  velWgt * cos(time * s.ySpeed + s.yPhase) * s.yAmp);
        }
    }

private:
};
