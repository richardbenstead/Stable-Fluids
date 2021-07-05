#pragma once
#include <iostream>
#include <string>

/* Scene Constants */
static const char *TITLE = "Stable Fluids";
constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

/* Simulator Constants */
constexpr int N = 16;
constexpr int SOURCE_SIZE = N/4;

constexpr float INTERACTION = 1000000.0f;

constexpr float VISCOSITY = 0.01f;
constexpr float GRAVITY_Y = 9.8f;
constexpr float DT = 0.05;
constexpr float FINISH_TIME = 3.0f;

constexpr int MATSIZE{N*N};
constexpr int SIZE{MATSIZE};

constexpr int POS(int i, int j) { return i + N * j; };

enum EMode
{
    E_Continuous = 0,
    E_Once = 1
};
