#pragma once

/* Scene Constants */
static const char *TITLE = "Stable Fluids";
constexpr int WIDTH = 1400;
constexpr int HEIGHT = 1000;

/* Simulator Constants */
constexpr int N = 100;
constexpr int SOURCE_SIZE = N/4;

constexpr float INTERACTION = 1000000.0f;

constexpr float VISCOSITY = 0.01f;
constexpr float GRAVITY_Y = 9.8f;
constexpr float DT = 0.0005;

constexpr int MATSIZE{N*N};

constexpr int POS(int i, int j) { return i + N * j; };

enum EMode
{
    E_Continuous = 0,
    E_Once = 1
};
