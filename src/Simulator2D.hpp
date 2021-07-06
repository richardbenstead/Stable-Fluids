#pragma once
#include <fftw3.h>
#include <glm/glm.hpp>
#include <iostream>

template<typename GridCellsType>
class Simulator2D
{
public:
    Simulator2D(GridCellsType& gridCells) :  mGridCells{gridCells}
    {
        m_fft_U = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * N * N);
        m_fft_V = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * N * N);
        m_plan_u_rc = fftwf_plan_dft_r2c_2d(N, N, mGridCells.u0, m_fft_U, FFTW_MEASURE);
        m_plan_v_rc = fftwf_plan_dft_r2c_2d(N, N, mGridCells.v0, m_fft_V, FFTW_MEASURE);
        m_plan_u_cr = fftwf_plan_dft_c2r_2d(N, N, m_fft_U, mGridCells.u0, FFTW_MEASURE);
        m_plan_v_cr = fftwf_plan_dft_c2r_2d(N, N, m_fft_V, mGridCells.v0, FFTW_MEASURE);
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
        for (int i = 0; i < MATSIZE; ++i) {
            mGridCells.fx[i] = 0.0f;
            mGridCells.fy[i] = GRAVITY_Y;
        }
        
        // advect density
        for (unsigned int j = 0; j < N; ++j) {
            for (unsigned int i = 0; i < N; ++i) {
                // identify source and interpolate
                float x = static_cast<float>(i) - N * DT * mGridCells.u[POS(i, j)];
                float y = static_cast<float>(j) - N * DT * mGridCells.v[POS(i, j)];
                mGridCells.dens[POS(i, j)] = interp(x, y, mGridCells.dens);
            }
        }
    }

private:
    void addForce() // adjust the velocities by the forces
    {
        for (int i = 0; i < MATSIZE; ++i) {
            mGridCells.u[i] += DT * mGridCells.fx[i];
            mGridCells.v[i] += DT * mGridCells.fy[i];

            mGridCells.u0[i] = mGridCells.u[i];
            mGridCells.v0[i] = mGridCells.v[i];
        }
    }

    void advect()
    {
        for (unsigned int j = 0; j < N; ++j) {
            for (unsigned int i = 1; i < N; ++i) {
                float x = i - N * DT * mGridCells.u0[POS(i, j)];
                float y = j - N * DT * mGridCells.v0[POS(i, j)];

                mGridCells.u[POS(i, j)] = interp(x, y, mGridCells.u0);
                mGridCells.v[POS(i, j)] = interp(x, y, mGridCells.v0);
            }
        }
    }

    void diffuse()
    {
        for (int i = 0; i < MATSIZE; ++i) { // copy velocity
            mGridCells.u0[i] = mGridCells.u[i];
            mGridCells.v0[i] = mGridCells.v[i];
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
        for (int i = 0; i < MATSIZE; ++i) {
            mGridCells.u[i] = f * mGridCells.u0[i];
            mGridCells.v[i] = f * mGridCells.v0[i];
        }
    }

    // interpolate the 4 points around the specified location
    float interp(const float x, const float y, const float q[])
    {
        const int intX = floor(x);
        const int intY = floor(y);
        const float decX = x - intX;
        const float decY = y - intY;

        auto inRange = [](float x) { return x >= 0 && x < N; };
        return ((inRange(intX) && inRange(intY)) ? q[POS(intX, intY)] * (1.0f - decX) * (1.0f - decY) : 0) +
               ((inRange(intX) && inRange(intY+1)) ? q[POS(intX, intY+1)] * (1.0f - decX) * decY : 0) +
               ((inRange(intX+1) && inRange(intY)) ? q[POS(intX+1, intY)] * decX * (1.0f - decY) : 0) +
               ((inRange(intX+1) && inRange(intY+1)) ? q[POS(intX+1, intY+1)] * decX * decY : 0);
    }


    GridCellsType& mGridCells;

    fftwf_plan m_plan_u_rc, m_plan_u_cr, m_plan_v_rc, m_plan_v_cr;
    fftwf_complex *m_fft_U;
    fftwf_complex *m_fft_V;
};
