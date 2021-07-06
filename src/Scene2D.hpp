#pragma once
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "constants.hpp"

class Scene2D
{
public:
  static void draw(const auto& gridCells)
  {
      glClear(GL_COLOR_BUFFER_BIT);
      glMatrixMode(GL_MODELVIEW);
      glViewport(0, 0, WIDTH, HEIGHT);
      glLoadIdentity();
      glOrtho(0, WIDTH, HEIGHT, 0, -1.0, 1.0);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      drawDensity(gridCells);
      //drawVelocity(gridCells);
  }

private:
  static void drawDensity(const auto& gridCells)
  {
      for (unsigned int y = 0; y < GRID_SIZE; ++y) {
          for (unsigned int x = 0; x < GRID_SIZE; ++x) {
              glColor4d(gridCells.densityR(x, y), gridCells.densityG(x, y), gridCells.densityB(x, y), 1.0f);
              glRectf(x * WIDTH / (float)GRID_SIZE, y * HEIGHT / (float)GRID_SIZE, (x + 1) * WIDTH / (float)GRID_SIZE, (y + 1) * HEIGHT / (float)GRID_SIZE);
          }
      }
  }

  static void drawVelocity(const auto& gridCells)
  {
      constexpr float ks{0.8f};
      glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
      glBegin(GL_LINES);
      for (unsigned int y = 0; y < GRID_SIZE; ++y) {
          for (unsigned int x = 0; x < GRID_SIZE; ++x) {
              glm::vec2 p = {(x + 0.5) * WIDTH / (float)GRID_SIZE, (y + 0.5) * HEIGHT / (float)GRID_SIZE};
              glm::vec2 vel = {gridCells.u[POS(x, y)], gridCells.v[POS(x, y)]};
              vel = glm::normalize(vel);
              glVertex2d(p.x, p.y);
              glVertex2d(p.x + ks * (WIDTH / (float)GRID_SIZE) * vel.x, p.y + ks * (HEIGHT / (float)GRID_SIZE) * vel.y);
          }
      }
      glEnd();
  }
};
