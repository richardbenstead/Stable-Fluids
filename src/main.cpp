#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "Scene2D.hpp"
#include "Simulator2D.hpp"
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    EMode mode = E_Continuous;
    for (int i=1; i < argc; ++i) {
        if (std::string(argv[i]) == "-o") {
            mode = E_Once;
        }
    }

    GridCells2D gridCells;
    Simulator2D simulator(gridCells, mode);

    // initialize OpenGL
    if (!glfwInit())
    {
        std::cerr << "glfwInit failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create Window
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "glfwCreateWindow failed" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &simulator);

    // register event callback function
    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
            Simulator2D* pSim = static_cast<Simulator2D*>(glfwGetWindowUserPointer(window));
            pSim->mouseEvent(window, button, action, mods); });
    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
            Simulator2D* pSim = static_cast<Simulator2D*>(glfwGetWindowUserPointer(window));
            pSim->mouseMoveEvent(window, xpos, ypos); });

    // initialize scene
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        simulator.update();
        Scene2D::draw(gridCells);

        // swap draw buffer
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

