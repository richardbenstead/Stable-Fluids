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
      drawVelocity(gridCells);
  }

private:
  static void drawDensity(const auto& gridCells)
  {
      for (unsigned int y = 0; y < N; ++y) {
          for (unsigned int x = 0; x < N; ++x) {
              glColor4d(1.0f, 1.0f, 1.0f, gridCells.dens[POS(x, y)]);
              glRectf(x * WIDTH / (float)N, y * HEIGHT / (float)N, (x + 1) * WIDTH / (float)N, (y + 1) * HEIGHT / (float)N);
          }
      }
  }

  static void drawVelocity(const auto& gridCells)
  {
      constexpr float ks{0.8f};
      glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
      glBegin(GL_LINES);
      for (unsigned int y = 0; y < N; ++y) {
          for (unsigned int x = 0; x < N; ++x) {
              glm::vec2 p = {(x + 0.5) * WIDTH / (float)N, (y + 0.5) * HEIGHT / (float)N};
              glm::vec2 vel = {gridCells.u[POS(x, y)], gridCells.v[POS(x, y)]};
              vel = glm::normalize(vel);
              glVertex2d(p.x, p.y);
              glVertex2d(p.x + ks * (WIDTH / (float)N) * vel.x, p.y + ks * (HEIGHT / (float)N) * vel.y);
          }
      }
      glEnd();
  }
};
