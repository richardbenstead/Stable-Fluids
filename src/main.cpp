#include "sceneMovingSources.h"
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

    StableFluids() : mSimulator(mGridCells), mScene(mGridCells), mWinFluid(mGridCells)
    {
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

            auto& scene = mScene;
            scene.update(time);
            mSimulator.update(scene.getGravity(), scene.getViscosity());

            mWinFluid.draw();
        }
    }

private:
    SimType mSimulator;
    GridCellsType mGridCells;
    SceneMovingSources<GridCellsType> mScene;
    WinFluidType mWinFluid;
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    /*
    for (int i=1; i < argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            mode = E_Once;
        }
    }*/

    StableFluids* sf = new StableFluids();
    sf->run();
    delete sf;

    return 0;
}
