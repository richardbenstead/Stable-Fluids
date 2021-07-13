#pragma once
#include <GL/gl.h>
#include <math.h>
#include <cstring>
#include <iostream>
#include <GLFW/glfw3.h>
#include "utils.h"


class GlWinBase
{
public:
    void initialize()
    {
        mpWindow = glfwCreateWindow(1200, 800, "Stable fluids sim", nullptr, nullptr);
        if (!mpWindow) {
            throw std::runtime_error("glfwCreateWindow failed");
        }

        glfwMakeContextCurrent(mpWindow);
        glfwSetWindowUserPointer(mpWindow, this);
    }

    virtual void draw() = 0;

protected:
    GLFWwindow *mpWindow{};
};

