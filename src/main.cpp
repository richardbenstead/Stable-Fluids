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
    using GridCellsType = GridCells2D<MATSIZE>;
    using SimType = Simulator2D<GridCellsType>;

    StableFluids() : mSimulator(mGridCells)
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
        addSource();
    }

    ~StableFluids()
    {
        glfwTerminate();
    }

    void run(EMode mode)
    {
        // register event callback function
        glfwSetMouseButtonCallback(mpWindow, [](GLFWwindow *window, int button, int action, int mods) {
                static_cast<StableFluids*>(glfwGetWindowUserPointer(window))->mouseEvent(window, button, action, mods); });
        glfwSetCursorPosCallback(mpWindow, [](GLFWwindow *window, double xpos, double ypos) {
                static_cast<StableFluids*>(glfwGetWindowUserPointer(window))->mouseMoveEvent(window, xpos, ypos); });

        // initialize scene
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        while (!glfwWindowShouldClose(mpWindow)) {
            mSimulator.update();
            if (mode == E_Continuous) addSource();

            Scene2D::draw(mGridCells);
            glfwSwapBuffers(mpWindow);
            glfwPollEvents();
        }
    }

    void addSource()
    {
        for (int j = N / 2 - SOURCE_SIZE / 2; j < N / 2 + SOURCE_SIZE / 2; ++j) {
            for (int i = N / 2 - SOURCE_SIZE / 2; i < N / 2 + SOURCE_SIZE / 2; ++i) {
                float& d = mGridCells.dens[POS(i, j)];
                d = std::min(1.0f, d+0.1f);
            }
        }
    }

    void mouseEvent(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            double px, py;
            glfwGetCursorPos(window, &px, &py);
            mLastMousePos = glm::ivec2(px, py);
            mMouseLeftDown = GLFW_PRESS==action;
        }
    }

    void mouseMoveEvent([[maybe_unused]] GLFWwindow *window, double xpos, double ypos)
    {
        if (mMouseLeftDown) {
            glm::ivec2 newMousePos = glm::ivec2(xpos, ypos);

            float dx = newMousePos.x - mLastMousePos.x;
            float dy = newMousePos.y - mLastMousePos.y;
            float length = dx * dx + dy * dy;
            if (length >= 2.0f) // ignore slight movement
            {
                // calculate force
                float tmp_fx = INTERACTION * N * (newMousePos.x - mLastMousePos.x) / (float)WIDTH;
                float tmp_fy = INTERACTION * N * (newMousePos.y - mLastMousePos.y) / (float)HEIGHT;

                // specify the index to add force
                unsigned int i = std::fmax(0.0, std::fmin(N - 1, N * newMousePos.x / (float)WIDTH));
                unsigned int j = std::fmax(0.0, std::fmin(N - 1, N * newMousePos.y / (float)HEIGHT));
                if (i > 0 && j > 0 && i < N - 1 && j < N - 1)
                { // avoid edge of grid
                    // calculate weight
                    float wx = N * newMousePos.x / (float)WIDTH - i;
                    float wy = N * newMousePos.y / (float)HEIGHT - j;

                    // add force
                    mGridCells.fx[POS(i, j)] = (1.0 - wx) * tmp_fx;
                    mGridCells.fx[POS(i + 1, j)] = wx * tmp_fx;
                    mGridCells.fy[POS(i, j)] = (1.0 - wy) * tmp_fy;
                    mGridCells.fy[POS(i, j + 1)] = wy * tmp_fy;
                }
                mLastMousePos = newMousePos;
            }
        }
    }

private:
    SimType mSimulator;
    GLFWwindow *mpWindow;
    GridCellsType mGridCells;

    bool mMouseLeftDown{false};
    glm::ivec2 mLastMousePos;
};

int main(int argc, char *argv[])
{
    EMode mode = E_Continuous;
    for (int i=1; i < argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            mode = E_Once;
        }
    }

    StableFluids sf;
    sf.run(mode);
    return 0;
}
