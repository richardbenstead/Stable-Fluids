#include "scene/sceneMovingSources.h"
#include "scene/sceneBlank.h"
#include "scene/sceneFire.h"
#include "scene/sceneText.h"
#include "simulator2D.h"
#include "gridCells2D.h"
#include <GLFW/glfw3.h>
#include "winFluid.h"
#include <stdexcept>
#include <iostream>
#include "utils.h"
#include <string>
#include <vector>

class StableFluids
{
public:
    using GridCellsType = GridCells2D<GRID_SIZE>;
    using SimType = Simulator2D<GridCellsType>;
    using WinFluidType = WinFluid<GridCellsType>;

    StableFluids() : mSimulator(mGridCells), mWinFluid(mGridCells)
    {
        mVecScene.push_back(new SceneMovingSources<GridCellsType>(mGridCells));
        mVecScene.push_back(new SceneFire<GridCellsType>(mGridCells));
        mVecScene.push_back(new SceneText<GridCellsType>(mGridCells));
        mVecScene.push_back(new SceneBlank<GridCellsType>(mGridCells));

        if (!glfwInit()) {
            throw std::runtime_error("glfwInit failed");
        }
        mWinFluid.initialize();
    }

    ~StableFluids()
    {
        glfwTerminate();
    }

    void run()
    {
        float time{};
        while (!mWinFluid.finished()) {
            time += DT;

            auto& scene = *mVecScene[mWinFluid.getSceneId() % mVecScene.size()];
            scene.update(time);
            mSimulator.update(scene.getParams());

            mWinFluid.draw();
        }
    }

private:
    SimType mSimulator;
    GridCellsType mGridCells;
    std::vector<SceneBase*> mVecScene;
    WinFluidType mWinFluid;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    StableFluids* sf = new StableFluids();
    sf->run();
    delete sf;

    return 0;
}
