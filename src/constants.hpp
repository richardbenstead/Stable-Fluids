#pragma once

/* Scene Constants */
static const char *TITLE = "Stable Fluids";
constexpr int WIDTH = 1400;
constexpr int HEIGHT = 1000;

/* Simulator Constants */
constexpr int GRID_SIZE = 500;

constexpr float INTERACTION = 1000000.0f;

constexpr float VISCOSITY = 0.01f;
constexpr float GRAVITY_Y = 1;
constexpr float DT = 0.001;

constexpr int POS(int i, int j) { return i + GRID_SIZE * j; };

// TODO:
// resize window
// key to change mode, grid etc
// don't render with rects
