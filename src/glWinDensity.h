#pragma once
#include "glWinBase.h"
#include <math.h>
#include <cstring>
#include <iostream>
#include "utils.h"


template<typename GridCellsType>
class GlWinDensity : public GlWinBase
{
    static constexpr uint16_t GRID_SIZE = GridCellsType::GRID_SIZE;
    auto POS(auto x, auto y) { return GridCellsType::POS(x,y); }
public:
    GlWinDensity(GridCellsType& gc) : mGridCells(gc)
    {}

    void initialize()
    {
        GlWinBase::initialize();

        // register event callback function
        glfwSetMouseButtonCallback(mpWindow, [](GLFWwindow *window, int button, int action, int mods) {
                static_cast<GlWinDensity*>(glfwGetWindowUserPointer(window))->mouseEvent(window, button, action, mods); });

        glfwSetCursorPosCallback(mpWindow, [](GLFWwindow *window, double xpos, double ypos) {
                static_cast<GlWinDensity*>(glfwGetWindowUserPointer(window))->mouseMoveEvent(window, xpos, ypos); });

        glfwSetKeyCallback(mpWindow, [](GLFWwindow* window, int key, int sc, int action, int mods) {
                static_cast<GlWinDensity*>(glfwGetWindowUserPointer(window))->keyEvent(window, key, sc, action, mods); });
    }

    void draw()
    {
        int width, height;
        glfwGetWindowSize(mpWindow, &width, &height);

        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glOrtho(0, width, 0, height, -1.0, 1.0);
        glViewport(0,0,width,height);

        drawDensity(width, height);
        //drawVelocity();
        
        glfwSwapBuffers(mpWindow);
        glfwPollEvents();
    }

    bool isFinished()
    {
        return mQuit || glfwWindowShouldClose(mpWindow);
    }

    int getSceneId() { return mSceneId; }

private:
    void drawDensity(const int width, const int height)
    {
        glPixelZoom(width/static_cast<float>(GRID_SIZE), -height/static_cast<float>(GRID_SIZE));
        glRasterPos2i(0, height);
        glDrawPixels(GRID_SIZE, GRID_SIZE, GL_RGB, GL_FLOAT, &mGridCells.density[0]);
    }

    void drawVelocity(const int width, const int height)
    {
        constexpr float ks{0.8f};
        glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        glBegin(GL_LINES);
        for (unsigned int y = 0; y < GRID_SIZE; ++y) {
            for (unsigned int x = 0; x < GRID_SIZE; ++x) {
                float px = (x + 0.5) * width / (float)GRID_SIZE;
                float py = (y + 0.5) * height / (float)GRID_SIZE;
                float vx = mGridCells.u[POS(x, y)];
                float vy = mGridCells.v[POS(x, y)];
                float len = sqrt(vx*vx + vy*vy);
                glVertex2d(px, py);
                glVertex2d(px + ks * (width / (float)GRID_SIZE) * vx/len, py + ks * (height / (float)GRID_SIZE) * vy/len);
            }
        }
        glEnd();
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
        constexpr float INTERACTION = 1000000.0f;
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

    void keyEvent([[maybe_unused]] GLFWwindow *window, int key, [[maybe_unused]] int sc, int action, [[maybe_unused]] int mods)
    {
        if (GLFW_PRESS == action) {
            if (key == 'Q') mQuit = true;
            if (key == ' ') mSceneId++;
        }
    }


    GridCellsType& mGridCells;

    int mSceneId{};
    bool mMouseLeftDown{false};
    XYPair mLastMousePos{};
    bool mQuit{false};
};
