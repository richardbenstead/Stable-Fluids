#pragma once
#include "GridCells2D.hpp"
#include <GL/gl.h>
#include <glm/glm.hpp>

class Scene2D
{
public:
  Scene2D(GridCells2D& grid_cells) : m_grid_cells(grid_cells) {}
  ~Scene2D(){}

  void draw()
  {
      glClear(GL_COLOR_BUFFER_BIT);
      glMatrixMode(GL_MODELVIEW);
      glViewport(0, 0, WIDTH, HEIGHT);
      glLoadIdentity();
      glOrtho(0, WIDTH, HEIGHT, 0, -1.0, 1.0);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      drawDensity();
      drawVelocity();
    }

private:
  void drawDensity()
  {
      for (unsigned int y = 0; y < N; ++y) {
          for (unsigned int x = 0; x < N; ++x) {
              glColor4d(1.0f, 1.0f, 1.0f, m_grid_cells.dens[POS(x, y)]);
              glRectf(x * WIDTH / (float)N, y * HEIGHT / (float)N, (x + 1) * WIDTH / (float)N, (y + 1) * HEIGHT / (float)N);
          }
      }
  }

  void drawVelocity()
  {
      constexpr float ks{0.8f};
      glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
      glBegin(GL_LINES);
      for (unsigned int y = 0; y < N; ++y) {
          for (unsigned int x = 0; x < N; ++x) {
              glm::vec2 p = {(x + 0.5) * WIDTH / (float)N, (y + 0.5) * HEIGHT / (float)N};
              glm::vec2 vel = {m_grid_cells.u[POS(x, y)], m_grid_cells.v[POS(x, y)]};
              vel = glm::normalize(vel);
              glVertex2d(p.x, p.y);
              glVertex2d(p.x + ks * (WIDTH / (float)N) * vel.x, p.y + ks * (HEIGHT / (float)N) * vel.y);
          }
      }
      glEnd();
  }

  GridCells2D& m_grid_cells;
};
