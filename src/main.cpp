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
    using GridCellsType = GridCells2D<GRID_SIZE>;
    using SimType = Simulator2D<GridCellsType>;
    using XYPair = typename GridCellsType::XYPair;

    StableFluids() : mSimulator(mGridCells)
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

        // initialize scene
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        while (!glfwWindowShouldClose(mpWindow)) {
            mTime += DT;
            mSimulator.update();

            auto getPoint = [&](float amp, float offset, float phase, float speed) {
                return sin(mTime * speed + phase) * amp + offset;
            };

            auto getSpeed = [&](float amp, float phase, float speed) {
                return cos(mTime * speed + phase) * amp;
            };

            float velWgt = DT * 15.0f;
            int size = std::max(1, GRID_SIZE/5);
            addBlock(getPoint(GRID_SIZE/2, GRID_SIZE/2, 0, 10),
                     getPoint(GRID_SIZE/2, GRID_SIZE/2, 0, 13),
                     size, size, 1, 0, 0,
                     velWgt * getSpeed(GRID_SIZE/3, 0, 10),
                     velWgt * getSpeed(GRID_SIZE/3, 0, 13));

            addBlock(getPoint(GRID_SIZE/2, GRID_SIZE/2, 1, 7),
                     getPoint(GRID_SIZE/2, GRID_SIZE/2, 1, 17),
                     size, size, 0, 1, 0,
                     velWgt * getSpeed(GRID_SIZE/3, 1, 7),
                     velWgt * getSpeed(GRID_SIZE/3, 1, 17));

            addBlock(getPoint(GRID_SIZE/2, GRID_SIZE/2, 2, 5),
                     getPoint(GRID_SIZE/2, GRID_SIZE/2, 2, 7),
                     size, size, 0, 0, 1,
                     velWgt * getSpeed(GRID_SIZE/3, 2, 5),
                     velWgt * getSpeed(GRID_SIZE/3, 2, 7));

            int width, height;
            glfwGetWindowSize(mpWindow, &width, &height);
            Scene2D::draw(mGridCells, width, height);
            glfwSwapBuffers(mpWindow);
            glfwPollEvents();
        }
    }

    void addBlock(const int x, const int y, const int width, const int height, const float r, const float g, const float b, const float u, const float v)
    {
        for(int i=-width/2; i<=width/2; ++i) {
            for(int j=-height/2; j<=height/2; ++j) {
                if (i+x>=0 && j+y>=0 && i+x<GRID_SIZE && j+y<GRID_SIZE) {
                    float alpha = -log(0.5)/(GRID_SIZE*GRID_SIZE/2000.0);
                    float wgt = exp(-alpha * (i*i + j*j));

                    auto satAdd = [](float& x, const float& y) { x = std::min(1.0f, x+y); };

                    satAdd(mGridCells.density[POS(i+x, j+y)].r, r*wgt);
                    satAdd(mGridCells.density[POS(i+x, j+y)].g, g*wgt);
                    satAdd(mGridCells.density[POS(i+x, j+y)].b, b*wgt);
                    mGridCells.velocity[POS(i+x, j+y)] += GridCellsType::XYPair{u*wgt, v*wgt};
                }
            }
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
    float mTime{};
    SimType mSimulator;
    GLFWwindow *mpWindow;
    GridCellsType mGridCells;

    bool mMouseLeftDown{false};
    XYPair mLastMousePos{};
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
