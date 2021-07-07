#pragma once

class SceneBase
{
public:
    virtual ~SceneBase() {};
    virtual void update(const float time) = 0;
    virtual float getViscosity() = 0;
    virtual float getGravity() { return 9.81f; }
    virtual float getPressureTrans() { return 1.0f; }
};
