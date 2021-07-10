#pragma once
#include "src/utils.h"

struct SceneParams
{
    float viscosity{};
    float gravity{};
    float densityTrans{};
    float diffusion{};
};

class SceneBase
{
public:
    virtual ~SceneBase() {};
    virtual void update(const float time) = 0;
    virtual SceneParams getParams() { return SceneParams{0.0001f, // viscosity
                                                           9.81, // gravity
                                                           0.9999f, // density
                                                           0.0f};} // diffusion
};
