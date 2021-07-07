#pragma once
#include <fftw3.h>
#include <iostream>

template<typename GridCellsType>
class Simulator2D
{
    using XYPair = typename GridCellsType::XYPair;
public:
    Simulator2D(GridCellsType& gridCells) :  mGridCells{gridCells}
    {
        mFft_uc = fftwf_alloc_complex(GRID_SIZE * GRID_SIZE);
        mFft_vc = fftwf_alloc_complex(GRID_SIZE * GRID_SIZE);
        mFft_ur = fftwf_alloc_real(GRID_SIZE * GRID_SIZE);
        mFft_vr = fftwf_alloc_real(GRID_SIZE * GRID_SIZE);
        m_plan_u_rc = fftwf_plan_dft_r2c_2d(GRID_SIZE, GRID_SIZE, mFft_ur, mFft_uc, FFTW_MEASURE);
        m_plan_v_rc = fftwf_plan_dft_r2c_2d(GRID_SIZE, GRID_SIZE, mFft_vr, mFft_vc, FFTW_MEASURE);
        m_plan_u_cr = fftwf_plan_dft_c2r_2d(GRID_SIZE, GRID_SIZE, mFft_uc, mFft_ur, FFTW_MEASURE);
        m_plan_v_cr = fftwf_plan_dft_c2r_2d(GRID_SIZE, GRID_SIZE, mFft_vc, mFft_vr, FFTW_MEASURE);
    }

    ~Simulator2D()
    {
        fftwf_destroy_plan(m_plan_u_rc);
        fftwf_destroy_plan(m_plan_v_rc);
        fftwf_destroy_plan(m_plan_u_cr);
        fftwf_destroy_plan(m_plan_v_cr);
        fftwf_free(mFft_uc);
        fftwf_free(mFft_vc);
        fftwf_free(mFft_ur);
        fftwf_free(mFft_vr);
    }

    template<typename DataType>
    void advect(const auto& velSource, const auto& dataSource, auto& dataTgt)
    {
        for (unsigned int j = 0; j < GRID_SIZE; ++j) {
            for (unsigned int i = 0; i < GRID_SIZE; ++i) {
                const int idx = POS(i, j);
                const XYPair point(i, j);
                const XYPair offset = velSource[idx] * GRID_SIZE * DT;
                dataTgt[idx] = interpolate<DataType>(point - offset, dataSource);
            }
        }
    }

    void update()
    {
        // Update velocities using forces
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
            mGridCells.velocity[i] += mGridCells.force[i] * DT;
            mGridCells.force[i] = XYPair{0.0f, GRAVITY_Y};
        }
        
        // Advect velocities
        mGridCells.velocityCopy = mGridCells.velocity;
        advect<XYPair>(mGridCells.velocityCopy, mGridCells.velocityCopy, mGridCells.velocity);
        diffuseVelocities();

        // advect density
        mGridCells.densityCopy = mGridCells.density;
        advect<typename GridCellsType::Density>(mGridCells.velocity, mGridCells.densityCopy, mGridCells.density);
    }

private:
    void diffuseVelocities()
    {
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) { // copy velocity
            mFft_ur[i] = mGridCells.velocity[i].x;
            mFft_vr[i] = mGridCells.velocity[i].y;
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

                    const float U0 = mFft_uc[idx][0];
                    const float V0 = mFft_vc[idx][0];
                    const float U1 = mFft_uc[idx][1];
                    const float V1 = mFft_vc[idx][1];

                    // update values, preserving mass
                    const float wxx = kx * kx / kk;
                    const float wxy = ky * kx / kk;
                    const float wyy = ky * ky / kk;
                    mFft_uc[idx][0] = f * ((1 - wxx) * U0 - wxy * V0);
                    mFft_uc[idx][1] = f * ((1 - wxx) * U1 - wxy * V1);
                    mFft_vc[idx][0] = f * (-wxy * U0 + (1 - wyy) * V0);
                    mFft_vc[idx][1] = f * (-wxy * U1 + (1 - wyy) * V1);
                }
            }
        }

        // convert back to real space
        fftwf_execute(m_plan_u_cr);
        fftwf_execute(m_plan_v_cr);

        // scale and copy back
        const float f = 1.0 / (float)(GRID_SIZE * GRID_SIZE);
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
            mGridCells.velocity[i] = XYPair(mFft_ur[i], mFft_vr[i]) * f;
        }
    }

    // interpolate the 4 cells around the specified point
    template<typename CellType>
    CellType interpolate(const XYPair& point, const auto& q)
    {
        const int intX = floor(point.x);
        const int intY = floor(point.y);
        const float decX = point.x - intX;
        const float decY = point.y - intY;

        auto inRange = [](const int x) { return x >= 0 && x < GRID_SIZE; };
        const bool xInRange = inRange(intX);
        const bool yInRange = inRange(intY);

        if (intX>=0 && intY>=0 && intX+1 < GRID_SIZE && intY+1 < GRID_SIZE) {
            [[likely]];
            return q[POS(intX, intY)] * (1.0f - decX) * (1.0f - decY) +
                   q[POS(intX, intY+1)] * (1.0f - decX) * decY +
                   q[POS(intX+1, intY)] * decX * (1.0f - decY) +
                   q[POS(intX+1, intY+1)] * decX * decY;
        }

        CellType out{};
        if (xInRange) {
            if (yInRange) {
                out += q[POS(intX, intY)] * (1.0f - decX) * (1.0f - decY);
            }
            if (inRange(intY+1)) {
                out += q[POS(intX, intY+1)] * (1.0f - decX) * decY;
            }
        }
        
        if (inRange(intX+1)) {
            if (yInRange) {
                out += q[POS(intX+1, intY)] * decX * (1.0f - decY);
            }
            if (inRange(intY+1)) {
                out += q[POS(intX+1, intY+1)] * decX * decY;
            }
        }
        return out;
    }

    GridCellsType& mGridCells;

    fftwf_plan m_plan_u_rc, m_plan_u_cr, m_plan_v_rc, m_plan_v_cr;
    fftwf_complex* mFft_uc;
    fftwf_complex* mFft_vc;
    float* mFft_ur;
    float* mFft_vr;
};
