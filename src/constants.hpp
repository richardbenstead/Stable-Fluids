#pragma once
/* Simulator Constants */
constexpr int GRID_SIZE = 200;

constexpr float INTERACTION = 1000000.0f;

constexpr float VISCOSITY = 0.01f;
constexpr float GRAVITY_Y = 2;
constexpr float DT = 0.001;

constexpr inline int POS(int i, int j) { return i + GRID_SIZE * j; };

// TODO:
// key to change mode, grid etc
// visualize FFT
// adjustable boundaries/barriers
// adjustable gravity
