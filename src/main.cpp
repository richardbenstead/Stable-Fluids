#include <GLFW/glfw3.h>
#include "WinFluid.h"
#include "Simulator2D.h"
#include "GridCells2D.h"
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "SceneMovingSources.h"

class StableFluids
{
public:
    using GridCellsType = GridCells2D<GRID_SIZE>;
    using SimType = Simulator2D<GridCellsType>;
    using XYPair = typename GridCellsType::XYPair;

    StableFluids() : mSimulator(mGridCells), mScene(mGridCells)
    {
        if (!glfwInit()) {
            throw std::runtime_error("glfwInit failed");
        }

        mpWindow = glfwCreateWindow(1200, 800, "Stable fluids sim", nullptr, nullptr);
        if (!mpWindow) {
            throw std::runtime_error("glfwCreateWindow failed");
        }

        glfwMakeContextCurrent(mpWindow);
        glfwSetWindowUserPointer(mpWindow, this);

    }

    ~StableFluids()
    {
        glfwTerminate();
    }

    void run()
    {
        // register event callback function
        glfwSetMouseButtonCallback(mpWindow, [](GLFWwindow *window, int button, int action, int mods) {
                static_cast<StableFluids*>(glfwGetWindowUserPointer(window))->mouseEvent(window, button, action, mods); });

        glfwSetCursorPosCallback(mpWindow, [](GLFWwindow *window, double xpos, double ypos) {
                static_cast<StableFluids*>(glfwGetWindowUserPointer(window))->mouseMoveEvent(window, xpos, ypos); });

        float time{};
        while (!glfwWindowShouldClose(mpWindow)) {
            time += DT;

            auto& scene = mScene;
            scene.update(time);
            mSimulator.update(scene.getGravity(), scene.getViscosity());

            int width, height;
            glfwGetWindowSize(mpWindow, &width, &height);
            WinFluid::draw(mGridCells, width, height);
            glfwSwapBuffers(mpWindow);
            glfwPollEvents();
        }
    }

    void mouseEvent(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            double px, py;
            glfwGetCursorPos(window, &px, &py);
            mLastMousePos = XYPair(px, py);
            mMouseLeftDown = GLFW_PRESS==action;
        }
    }

    void mouseMoveEvent([[maybe_unused]] GLFWwindow *window, double xpos, double ypos)
    {
        if (mMouseLeftDown) {
            int width, height;
            glfwGetWindowSize(mpWindow, &width, &height);

            XYPair newMousePos(xpos, ypos);
            XYPair delta = newMousePos - mLastMousePos;
            if (delta.norm() >= 2.0f) // ignore slight movement
            {
                int idx = POS(std::max(0, std::min<int>(GRID_SIZE-1, GRID_SIZE * newMousePos.x / (float)width)),
                              std::max(0, std::min<int>(GRID_SIZE-1, GRID_SIZE * newMousePos.y / (float)height)));

                mGridCells.force[idx] = XYPair{GRID_SIZE * delta.x / width,
                                               GRID_SIZE * delta.y / height} * INTERACTION;
                mLastMousePos = newMousePos;
            }
        }
    }

private:
    SimType mSimulator;
    GLFWwindow *mpWindow;
    GridCellsType mGridCells;

    bool mMouseLeftDown{false};
    XYPair mLastMousePos{};
    SceneMovingSources<GridCellsType> mScene;
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
