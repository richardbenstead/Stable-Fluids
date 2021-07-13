#pragma once
#include "src/scene/sceneBase.h"
#include <vector>


template<typename GridCellsType>
class SceneMovingSources : public SceneBase<GridCellsType>
{
    using baseType = SceneBase<GridCellsType>;
    auto POS(auto x, auto y) { return GridCellsType::POS(x,y); }
    static constexpr uint16_t GRID_SIZE = GridCellsType::GRID_SIZE;
public:
    SceneMovingSources(GridCellsType& gc) : baseType(gc) {}

    constexpr SceneParams getParams() { return SceneParams{0.001f, // viscosity
                                                           9.0f, // gravity
                                                           0.999f, // density
                                                           0.00001f};} // diffusion

    void update(const float time)
    {
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
            baseType::addGaussian(sin(time * s.xSpeed + s.xPhase) * s.xAmp + s.xOffset,
                                  sin(time * s.ySpeed + s.yPhase) * s.yAmp + s.yOffset,
                                  size, size, s.r, s.g, s.b,
                                  velWgt * cos(time * s.xSpeed + s.xPhase) * s.xAmp,
                                  velWgt * cos(time * s.ySpeed + s.yPhase) * s.yAmp);
        }
    }

private:
};
