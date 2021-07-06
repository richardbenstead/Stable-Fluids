#pragma once
#include <fftw3.h>
#include <glm/glm.hpp>

template<typename GridCellsType>
class Simulator2D
{
public:
  Simulator2D(GridCellsType& gridCells, EMode mode) :  m_gridCells{gridCells}, m_mode(mode)
  {
      m_fft_U = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * N * N);
      m_fft_V = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * N * N);
      m_plan_u_rc = fftwf_plan_dft_r2c_2d(N, N, m_gridCells.u0, m_fft_U, FFTW_MEASURE);
      m_plan_v_rc = fftwf_plan_dft_r2c_2d(N, N, m_gridCells.v0, m_fft_V, FFTW_MEASURE);
      m_plan_u_cr = fftwf_plan_dft_c2r_2d(N, N, m_fft_U, m_gridCells.u0, FFTW_MEASURE);
      m_plan_v_cr = fftwf_plan_dft_c2r_2d(N, N, m_fft_V, m_gridCells.v0, FFTW_MEASURE);

      addSource();
  }

  ~Simulator2D()
  {
      fftwf_destroy_plan(m_plan_u_rc);
      fftwf_destroy_plan(m_plan_v_rc);
      fftwf_destroy_plan(m_plan_u_cr);
      fftwf_destroy_plan(m_plan_v_cr);
      fftwf_free(m_fft_U);
      fftwf_free(m_fft_V);
  }

  void update()
  {
      // velocity step
      addForce();
      advect();
      diffuse();

      // density step
      for (int i = 0; i < SIZE; ++i) {
          m_gridCells.fx[i] = 0.0f;
          m_gridCells.fy[i] = GRAVITY_Y;
      }
      
      // advect density
      for (unsigned int j = 0; j < N; ++j) {
          for (unsigned int i = 0; i < N; ++i) {
              float x0 = i * WIDTH / (float)N;
              float y0 = j * HEIGHT / (float)N;

              float x = x0 - DT * interp(x0, y0, m_gridCells.u, N, N);
              float y = y0 - DT * interp(x0, y0, m_gridCells.v, N, N);

              m_gridCells.dens[POS(i, j)] = interp(x, y, m_gridCells.dens, N, N);
          }
      }

      if (m_mode == E_Continuous) {
          addSource();
      }
  }

  void mouseEvent(GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
  {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
          double px, py;
          glfwGetCursorPos(window, &px, &py);

          m_is_dragging = GLFW_PRESS==action;
          m_old_pos = glm::ivec2(px, py);
          m_new_pos = glm::ivec2(px, py);
      }
  }

  void mouseMoveEvent([[maybe_unused]] GLFWwindow *window, double xpos, double ypos)
  {
      if (m_is_dragging) {
          m_new_pos = glm::ivec2(xpos, ypos);

          float dx = m_new_pos.x - m_old_pos.x;
          float dy = m_new_pos.y - m_old_pos.y;
          float length = dx * dx + dy * dy;
          if (length >= 2.0f) // ignore slight movement
          {
              // calculate force
              float tmp_fx = INTERACTION * N * (m_new_pos.x - m_old_pos.x) / (float)WIDTH;
              float tmp_fy = INTERACTION * N * (m_new_pos.y - m_old_pos.y) / (float)HEIGHT;

              // specify the index to add force
              unsigned int i = std::fmax(0.0, std::fmin(N - 1, N * m_new_pos.x / (float)WIDTH));
              unsigned int j = std::fmax(0.0, std::fmin(N - 1, N * m_new_pos.y / (float)HEIGHT));
              if (i > 0 && j > 0 && i < N - 1 && j < N - 1)
              { // avoid edge of grid
                  // calculate weight
                  float wx = N * m_new_pos.x / (float)WIDTH - i;
                  float wy = N * m_new_pos.y / (float)HEIGHT - j;

                  // add force
                  m_gridCells.fx[POS(i, j)] = (1.0 - wx) * tmp_fx;
                  m_gridCells.fx[POS(i + 1, j)] = wx * tmp_fx;
                  m_gridCells.fy[POS(i, j)] = (1.0 - wy) * tmp_fy;
                  m_gridCells.fy[POS(i, j + 1)] = wy * tmp_fy;
              }
              m_old_pos = m_new_pos;
          }
      }
  }

private:
  void addForce() // adjust the velocities by the forces
  {
      for (int i = 0; i < SIZE; ++i) {
          m_gridCells.u[i] += DT * m_gridCells.fx[i];
          m_gridCells.v[i] += DT * m_gridCells.fy[i];

          m_gridCells.u0[i] = m_gridCells.u[i];
          m_gridCells.v0[i] = m_gridCells.v[i];
      }
  }

  void advect()
  {
      for (unsigned int j = 0; j < N; ++j) {
          for (unsigned int i = 1; i < N; ++i) {
              float x = i * WIDTH / (float)N;
              float y = (j + 0.5) * HEIGHT / (float)N;

              x = x - DT * interp(x, y - 0.5 * HEIGHT / (float)N, m_gridCells.u0, N + 1, N);
              y = y - DT * interp(x - 0.5 * WIDTH / (float)N, y, m_gridCells.v0, N, N + 1);

              m_gridCells.u[POS(i, j)] = interp(x, y - 0.5 * HEIGHT / (float)N, m_gridCells.u0, N + 1, N);
          }
      }

      for (unsigned int j = 1; j < N; ++j) {
          for (unsigned int i = 0; i < N; ++i) {
              float x = (i + 0.5) * WIDTH / (float)N;
              float y = j * HEIGHT / (float)N;

              x = x - DT * interp(x, y - 0.5 * HEIGHT / (float)N, m_gridCells.u0, N + 1, N);
              y = y - DT * interp(x - 0.5 * WIDTH / (float)N, y, m_gridCells.v0, N, N + 1);

              m_gridCells.v[POS(i, j)] = interp(x - 0.5 * WIDTH / (float)N, y, m_gridCells.v0, N, N + 1);
          }
      }
  }

  void diffuse()
  {
      for (int i = 0; i < SIZE; ++i) { // copy velocity
          m_gridCells.u0[i] = m_gridCells.u[i];
          m_gridCells.v0[i] = m_gridCells.v[i];
      }

      fftwf_execute(m_plan_u_rc); // FFT of velocities
      fftwf_execute(m_plan_v_rc);

      // diffuse step in fourier space
      // damp viscosity and conserve mass
      for (int j = 0; j < N; ++j) {
          float ky = (j <= N / 2) ? j : j - N;
          for (int i = 0; i <= N / 2; ++i) {
              float kx = i;
              float kk = kx * kx + ky * ky;

              if (kk > 0.001)
              {
                  float f = std::exp(-kk * DT * VISCOSITY);
                  int idx = i + j * (N / 2 + 1);
                  float U0 = m_fft_U[idx][0];
                  float V0 = m_fft_V[idx][0];

                  float U1 = m_fft_U[idx][1];
                  float V1 = m_fft_V[idx][1];

                  m_fft_U[idx][0] = f * ((1 - kx * kx / kk) * U0 - (kx * ky / kk) * V0);
                  m_fft_U[idx][1] = f * ((1 - kx * kx / kk) * U1 - (kx * ky / kk) * V1);
                  m_fft_V[idx][0] = f * ((-kx * ky / kk) * U0 + (1 - ky * ky / kk) * V0);
                  m_fft_V[idx][1] = f * ((-kx * ky / kk) * U1 + (1 - ky * ky / kk) * V1);
              }
          }
      }

      // convert back to real space
      fftwf_execute(m_plan_u_cr);
      fftwf_execute(m_plan_v_cr);

      // scale and copy back
      float f = 1.0 / (float)(N * N);
      for (int i = 0; i < SIZE; ++i) {
          m_gridCells.u[i] = f * m_gridCells.u0[i];
          m_gridCells.v[i] = f * m_gridCells.v0[i];
      }
  }

  void addSource()
  {
      for (int j = N / 2 - SOURCE_SIZE / 2; j < N / 2 + SOURCE_SIZE / 2; ++j) {
          for (int i = N / 2 - SOURCE_SIZE / 2; i < N / 2 + SOURCE_SIZE / 2; ++i) {
              float& d = m_gridCells.dens[POS(i, j)];
              d = std::min(1.0f, d+0.01f);
          }
      }
  }

  float interp(float x, float y, float q[], unsigned int Nx, unsigned int Ny)
  {
      x = std::fmax(0.0, std::fmin(Nx - 1 - 1e-6, N * x / (float)WIDTH));
      y = std::fmax(0.0, std::fmin(Ny - 1 - 1e-6, N * y / (float)HEIGHT));

      unsigned int i = x;
      unsigned int j = y;

      // take the 4 points around the specified location
      float f[4] = {q[POS(i, j)], q[POS(i, j + 1)], q[POS(i + 1, j)], q[POS(i + 1, j + 1)]};

      x = x - i;
      y = y - j;

      // interpolation coefficients
      float c[4] = {(1.0f - x) * (1.0f - y), (1.0f - x) * y, x * (1.0f - y), x * y};

      return c[0] * f[0] + c[1] * f[1] + c[2] * f[2] + c[3] * f[3];
  }


  GridCellsType& m_gridCells;

  fftwf_plan m_plan_u_rc, m_plan_u_cr, m_plan_v_rc, m_plan_v_cr;
  fftwf_complex *m_fft_U;
  fftwf_complex *m_fft_V;

  EMode m_mode;
  bool m_is_dragging;
  glm::ivec2 m_new_pos;
  glm::ivec2 m_old_pos;
};
