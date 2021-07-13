#pragma once
#include "src/scene/sceneBase.h"


template<typename GridCellsType>
class SceneBlank : public SceneBase<GridCellsType>
{
public:
    SceneBlank(GridCellsType& gc) : SceneBase<GridCellsType>(gc) {}

    constexpr SceneParams getParams() { return SceneParams{0, // viscosity
                                                           20, // gravity
                                                           0.99, // density
                                                           0.05f};} // diffusion

    void update([[maybe_unused]] const float time)
    {
    }

private:
};

