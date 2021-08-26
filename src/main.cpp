#include "scene/sceneMovingSources.h"
#include "scene/sceneBlank.h"
#include "scene/sceneFire.h"
#include "scene/sceneText.h"
#include "simulator2D.h"
#include "gridCells2D.h"
#include <GLFW/glfw3.h>
#include "glWinDensity.h"
#include <stdexcept>
#include <iostream>
#include "utils.h"
#include <string>
#include <vector>


class StableFluids
{
    static constexpr int GRID_SIZE{350};
    static constexpr float DT{0.001f};
public:
    using GridCellsType = GridCells2D<GRID_SIZE>;
    using SimType = Simulator2D<GridCellsType>;
    using WinDensityType = GlWinDensity<GridCellsType>;

    StableFluids() : mSimulator(mGridCells, DT), mWinDensity(mGridCells)
    {
        mVecScene.push_back(new SceneMovingSources<GridCellsType>(mGridCells));
        mVecScene.push_back(new SceneFire<GridCellsType>(mGridCells));
        mVecScene.push_back(new SceneText<GridCellsType>(mGridCells));
        mVecScene.push_back(new SceneBlank<GridCellsType>(mGridCells));

        if (!glfwInit()) {
            throw std::runtime_error("glfwInit failed");
        }
        mWinDensity.initialize();
    }

    ~StableFluids()
    {
        glfwTerminate();
    }

    void run()
    {
        float time{};
        while (!mWinDensity.isFinished()) {
            time += DT;

            auto& scene = *mVecScene[mWinDensity.getSceneId() % mVecScene.size()];
            scene.update(time);
            mSimulator.update(scene.getParams());

            mWinDensity.draw();
        }
    }

private:
    SimType mSimulator;
    GridCellsType mGridCells;
    std::vector<SceneBase<GridCellsType>*> mVecScene;
    WinDensityType mWinDensity;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    StableFluids* sf = new StableFluids();
    sf->run();
    delete sf;

    return 0;
}
