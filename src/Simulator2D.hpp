#pragma once
#include <fftw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <eigen3/Eigen/Dense>

template<typename GridCellsType>
class Simulator2D
{
public:
    Simulator2D(GridCellsType& gridCells) :  mGridCells{gridCells}
    {
        m_fft_U = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * GRID_SIZE * GRID_SIZE);
        m_fft_V = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * GRID_SIZE * GRID_SIZE);
        m_plan_u_rc = fftwf_plan_dft_r2c_2d(GRID_SIZE, GRID_SIZE, mGridCells.u0, m_fft_U, FFTW_MEASURE);
        m_plan_v_rc = fftwf_plan_dft_r2c_2d(GRID_SIZE, GRID_SIZE, mGridCells.v0, m_fft_V, FFTW_MEASURE);
        m_plan_u_cr = fftwf_plan_dft_c2r_2d(GRID_SIZE, GRID_SIZE, m_fft_U, mGridCells.u0, FFTW_MEASURE);
        m_plan_v_cr = fftwf_plan_dft_c2r_2d(GRID_SIZE, GRID_SIZE, m_fft_V, mGridCells.v0, FFTW_MEASURE);
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
        // Update velocities using forces
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
            mGridCells.u[i] += DT * mGridCells.fx[i];
            mGridCells.v[i] += DT * mGridCells.fy[i];

            // reset forces
            mGridCells.fx[i] = 0.0f;
            mGridCells.fy[i] = GRAVITY_Y;

            // copy velocities
            mGridCells.u0[i] = mGridCells.u[i];
            mGridCells.v0[i] = mGridCells.v[i];
        }
        
        // Advect velocities
        for (unsigned int j = 0; j < GRID_SIZE; ++j) {
            for (unsigned int i = 1; i < GRID_SIZE; ++i) {
                const float x = i - GRID_SIZE * DT * mGridCells.u0[POS(i, j)];
                const float y = j - GRID_SIZE * DT * mGridCells.v0[POS(i, j)];

                /*
                auto xu0 = Eigen::Map<typename GridCellsType::MatCellsType>(mGridCells.u0);
                auto xv0 = Eigen::Map<typename GridCellsType::MatCellsType>(mGridCells.v0);
                auto xu = Eigen::Map<typename GridCellsType::MatCellsType>(mGridCells.u);
                auto xv = Eigen::Map<typename GridCellsType::MatCellsType>(mGridCells.v);
                xu(i, j) = interpolate2(x, y, xu0);
                xv(i, j) = interpolate2(x, y, xv0);
                */
                mGridCells.u[POS(i, j)] = interpolate(x, y, mGridCells.u0);
                mGridCells.v[POS(i, j)] = interpolate(x, y, mGridCells.v0);
            }
        }
        diffuseVelocities();

        // advect density
        mGridCells.densityCopyR = mGridCells.densityR;
        mGridCells.densityCopyG = mGridCells.densityG;
        mGridCells.densityCopyB = mGridCells.densityB;
        for (unsigned int j = 0; j < GRID_SIZE; ++j) {
            for (unsigned int i = 0; i < GRID_SIZE; ++i) {
                float x = i - GRID_SIZE * DT * mGridCells.u[POS(i, j)];
                float y = j - GRID_SIZE * DT * mGridCells.v[POS(i, j)];
                mGridCells.densityR(i, j) = interpolate2(x, y, mGridCells.densityR);
                mGridCells.densityG(i, j) = interpolate2(x, y, mGridCells.densityG);
                mGridCells.densityB(i, j) = interpolate2(x, y, mGridCells.densityB);
            }
        }
    }

private:
    void diffuseVelocities()
    {
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) { // copy velocity
            mGridCells.u0[i] = mGridCells.u[i];
            mGridCells.v0[i] = mGridCells.v[i];
        }

        fftwf_execute(m_plan_u_rc); // FFT of velocities
        fftwf_execute(m_plan_v_rc);

        // diffuse step in fourier space
        // damp viscosity and conserve mass
        for (int j = 0; j < GRID_SIZE; ++j) {
            const float ky = (j <= GRID_SIZE / 2) ? j : j - GRID_SIZE;
            for (int i = 0; i <= GRID_SIZE / 2; ++i) {
                const float kx = i;
                const float kk = kx * kx + ky * ky; // squared norm

                if (kk > 0.001)
                {
                    const float f = std::exp(-kk * DT * VISCOSITY);
                    const int idx = i + j * (GRID_SIZE / 2 + 1);

                    const float U0 = m_fft_U[idx][0];
                    const float V0 = m_fft_V[idx][0];
                    const float U1 = m_fft_U[idx][1];
                    const float V1 = m_fft_V[idx][1];

                    // update values, preserving mass
                    const float wxx = kx * kx / kk;
                    const float wxy = ky * kx / kk;
                    const float wyy = ky * ky / kk;
                    m_fft_U[idx][0] = f * ((1 - wxx) * U0 - wxy * V0);
                    m_fft_U[idx][1] = f * ((1 - wxx) * U1 - wxy * V1);
                    m_fft_V[idx][0] = f * (-wxy * U0 + (1 - wyy) * V0);
                    m_fft_V[idx][1] = f * (-wxy * U1 + (1 - wyy) * V1);
                }
            }
        }

        // convert back to real space
        fftwf_execute(m_plan_u_cr);
        fftwf_execute(m_plan_v_cr);

        // scale and copy back
        const float f = 1.0 / (float)(GRID_SIZE * GRID_SIZE);
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
            mGridCells.u[i] = f * mGridCells.u0[i];
            mGridCells.v[i] = f * mGridCells.v0[i];
        }
    }

    float interpolate(const float x, const float y, const float q[])
    {
        const int intX = floor(x);
        const int intY = floor(y);
        const float decX = x - intX;
        const float decY = y - intY;

        auto inRange = [](float x) { return x >= 0 && x < GRID_SIZE; };
        return ((inRange(intX) && inRange(intY)) ? q[POS(intX, intY)] * (1.0f - decX) * (1.0f - decY) : 0) +
               ((inRange(intX) && inRange(intY+1)) ? q[POS(intX, intY+1)] * (1.0f - decX) * decY : 0) +
               ((inRange(intX+1) && inRange(intY)) ? q[POS(intX+1, intY)] * decX * (1.0f - decY) : 0) +
               ((inRange(intX+1) && inRange(intY+1)) ? q[POS(intX+1, intY+1)] * decX * decY : 0);
    }

    // interpolate the 4 points around the specified location
    float interpolate2(const float x, const float y, const GridCellsType::MatCellsType& q)
    {
        const int intX = floor(x);
        const int intY = floor(y);
        const float decX = x - intX;
        const float decY = y - intY;

        auto inRange = [](float x) { return x >= 0 && x < GRID_SIZE; };
        return ((inRange(intX) && inRange(intY)) ? q(intX, intY) * (1.0f - decX) * (1.0f - decY) : 0) +
               ((inRange(intX) && inRange(intY+1)) ? q(intX, intY+1) * (1.0f - decX) * decY : 0) +
               ((inRange(intX+1) && inRange(intY)) ? q(intX+1, intY) * decX * (1.0f - decY) : 0) +
               ((inRange(intX+1) && inRange(intY+1)) ? q(intX+1, intY+1) * decX * decY : 0);
    }


    GridCellsType& mGridCells;

    fftwf_plan m_plan_u_rc, m_plan_u_cr, m_plan_v_rc, m_plan_v_cr;
    fftwf_complex *m_fft_U;
    fftwf_complex *m_fft_V;
};
