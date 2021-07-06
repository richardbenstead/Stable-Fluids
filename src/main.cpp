#include <GLFW/glfw3.h>
#include "Scene2D.hpp"
#include "Simulator2D.hpp"
#include "GridCells2D.hpp"
#include <string>
#include <iostream>
#include <stdexcept>

class StableFluids
{
public:
    using GridCellsType = GridCells2D<SIZE>;
    using SimType = Simulator2D<GridCellsType>;

    StableFluids(EMode mode) : mSimulator(mGridCells, mode)
    {
        if (!glfwInit()) {
            throw std::runtime_error("glfwInit failed");
        }

        mpWindow = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
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
                StableFluids* pSF = static_cast<StableFluids*>(glfwGetWindowUserPointer(window));
                pSF->mSimulator.mouseEvent(window, button, action, mods); });
        glfwSetCursorPosCallback(mpWindow, [](GLFWwindow *window, double xpos, double ypos) {
                StableFluids* pSF = static_cast<StableFluids*>(glfwGetWindowUserPointer(window));
                pSF->mSimulator.mouseMoveEvent(window, xpos, ypos); });

        // initialize scene
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        while (!glfwWindowShouldClose(mpWindow)) {
            mSimulator.update();
            Scene2D::draw(mGridCells);
            glfwSwapBuffers(mpWindow);
            glfwPollEvents();
        }
    }

    SimType mSimulator;
private:
    GLFWwindow *mpWindow;
    GridCellsType mGridCells;
};

int main(int argc, char *argv[])
{
    EMode mode = E_Continuous;
    for (int i=1; i < argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            mode = E_Once;
        }
    }

    StableFluids sf(mode);
    sf.run();
    return 0;
}
