#pragma once
#include <GL/gl.h>
#include "constants.h"
#include <math.h>
#include <cstring>
#include <iostream>

class WinFluid
{
public:
  static void draw(const auto& gridCells, const int width, const int height)
  {
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glOrtho(0, width, 0, height, -1.0, 1.0);
      glViewport(0,0,width,height);

      drawDensity(gridCells, width, height);
      //drawVelocity(gridCells);
  }

private:
  static void drawDensity(const auto& gridCells, const int width, const int height)
  {
      glPixelZoom(width/static_cast<float>(GRID_SIZE), -height/static_cast<float>(GRID_SIZE));
      glRasterPos2i(0, height);
      glDrawPixels(GRID_SIZE, GRID_SIZE, GL_RGB, GL_FLOAT, &gridCells.density[0]);
  }

  static void drawVelocity(const auto& gridCells, const int width, const int height)
  {
      constexpr float ks{0.8f};
      glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
      glBegin(GL_LINES);
      for (unsigned int y = 0; y < GRID_SIZE; ++y) {
          for (unsigned int x = 0; x < GRID_SIZE; ++x) {
              float px = (x + 0.5) * width / (float)GRID_SIZE;
              float py = (y + 0.5) * height / (float)GRID_SIZE;
              float vx = gridCells.u[POS(x, y)];
              float vy = gridCells.v[POS(x, y)];
              float len = sqrt(vx*vx + vy*vy);
              glVertex2d(px, py);
              glVertex2d(px + ks * (width / (float)GRID_SIZE) * vx/len, py + ks * (height / (float)GRID_SIZE) * vy/len);
          }
      }
      glEnd();
  }
};
