#pragma once
#include <fftw3.h>
#include <iostream>

template<typename GridCellsType>
class Simulator2D
{
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
        for (unsigned int j = 1; j < GRID_SIZE-1; ++j) {
            for (unsigned int i = 1; i < GRID_SIZE-1; ++i) {
                const int idx = POS(i, j);
                XYPair point = XYPair(i, j) - velSource[idx] * GRID_SIZE * DT;
                dataTgt[idx] = interpolate<DataType>(point, dataSource);
            }
        }
    }

    void setDensityBoundary(auto& dataTgt)
    {
        for (int i=1 ; i<=GRID_SIZE ; i++) {
            dataTgt[POS(0, i)] = dataTgt[POS(1, i)];
            dataTgt[POS(GRID_SIZE-1, i)] = dataTgt[POS(GRID_SIZE-2,i)];
            dataTgt[POS(i, 0)] = dataTgt[POS(i, 1)];
            dataTgt[POS(i, GRID_SIZE-1)] = dataTgt[POS(i,GRID_SIZE-2)];
        }
        dataTgt[POS(0, 0)] = (dataTgt[POS(1, 0)]+dataTgt[POS(0, 1)]) * 0.5;
        dataTgt[POS(0, GRID_SIZE-1)] = (dataTgt[POS(1, GRID_SIZE-1)]+dataTgt[POS(0, GRID_SIZE-2)]) * 0.5;
        dataTgt[POS(GRID_SIZE-1, 0)] = (dataTgt[POS(GRID_SIZE-2, 0)]+dataTgt[POS(GRID_SIZE-1, 1)]) * 0.5;
        dataTgt[POS(GRID_SIZE-1, GRID_SIZE-1)] = (dataTgt[POS(GRID_SIZE-2, GRID_SIZE-1)]+dataTgt[POS(GRID_SIZE-1, GRID_SIZE-2)]) * 0.5;
    }

    void setVelocityBoundary(auto& dataTgt)
    {
        for (int i=1; i<=GRID_SIZE; i++) {
            dataTgt[POS(0, i)].x = -dataTgt[POS(1,i)].x;
            dataTgt[POS(0, i)].y = dataTgt[POS(1,i)].y;
            dataTgt[POS(GRID_SIZE-1, i)].x = 0-dataTgt[POS(GRID_SIZE-2,i)].x;
            dataTgt[POS(GRID_SIZE-1, i)].y = dataTgt[POS(GRID_SIZE-2,i)].y;
            dataTgt[POS(i, 0)].x = dataTgt[POS(i,1)].x;
            dataTgt[POS(i, 0)].y = -dataTgt[POS(i,1)].y;
            dataTgt[POS(i, GRID_SIZE-1)].x = dataTgt[POS(i,GRID_SIZE-2)].x;
            dataTgt[POS(i, GRID_SIZE-1)].y = -dataTgt[POS(i,GRID_SIZE-2)].y;
        }
        dataTgt[POS(0,0)] = (dataTgt[POS(1,0)]+dataTgt[POS(0,1)]) * 0.5;
        dataTgt[POS(0, GRID_SIZE-1)] = (dataTgt[POS(1, GRID_SIZE-1)]+dataTgt[POS(0, GRID_SIZE-2)]) * 0.5;
        dataTgt[POS(GRID_SIZE-1,0)] = (dataTgt[POS(GRID_SIZE-2, 0)]+dataTgt[POS(GRID_SIZE-1,1)]) * 0.5;
        dataTgt[POS(GRID_SIZE-1,GRID_SIZE-1)] = (dataTgt[POS(GRID_SIZE-2, GRID_SIZE-1)]+dataTgt[POS(GRID_SIZE-1, GRID_SIZE-2)]) * 0.5;
    }

    void diffuse(auto& dataTgt, const auto& dataSource, const float diffusion, const float trans)
    {
        const float a = DT * diffusion * GRID_SIZE * GRID_SIZE;
        for (int k=0 ; k<20 ; k++ ) {
            for (int i=1 ; i<=GRID_SIZE-2 ; i++ ) {
                for (int j=1 ; j<=GRID_SIZE-2 ; j++ ) {
                    dataTgt[POS(i,j)] = (dataSource[POS(i,j)] + (dataTgt[POS(i-1,j)] + dataTgt[POS(i+1,j)] +
                                         dataTgt[POS(i,j-1)] + dataTgt[POS(i,j+1)]) * a) * (trans/(1+4*a));
                }
            }
            setDensityBoundary(dataTgt);
        }
    }

    void update(const auto params)
    {
        // Update velocities using forces
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) {
            mGridCells.velocity[i] += mGridCells.force[i] * DT;
            mGridCells.force[i] = XYPair{0.0f, params.gravity};
        }
        
        // apply viscosity term and solve for non-divergent velocities
        // setVelocityBoundary(mGridCells.velocity);
        diffuseVelocities(params.viscosity);
        setVelocityBoundary(mGridCells.velocity);

        // Advect density
        mGridCells.densityCopy = mGridCells.density;
        advect<Density>(mGridCells.velocity, mGridCells.densityCopy, mGridCells.density);

        mGridCells.densityCopy = mGridCells.density;
        diffuse(mGridCells.density, mGridCells.densityCopy, params.diffusion, params.densityTrans);

        // Advect velocities
        mGridCells.velocityCopy = mGridCells.velocity;
        advect<XYPair>(mGridCells.velocityCopy, mGridCells.velocityCopy, mGridCells.velocity);
    }

private:
    void diffuseVelocities(const float viscosity)
    {
        for (int i = 0; i < GRID_SIZE*GRID_SIZE; ++i) { // copy velocity
            const auto& v = mGridCells.velocity[i];
            mFft_ur[i] = v.x;
            mFft_vr[i] = v.y;
        }

        fftwf_execute(m_plan_u_rc); // FFT of velocities
        fftwf_execute(m_plan_v_rc);

        // diffuse step in frequency domain
        for (int j = 0; j < GRID_SIZE; ++j) {
            int idx = j * (GRID_SIZE / 2 + 1);
            const float ky = (j <= GRID_SIZE / 2) ? j : j - GRID_SIZE;
            for (int i = 0; i <= GRID_SIZE / 2; ++i) {
                const float kx = i;
                const float kk = kx * kx + ky * ky; // squared norm

                if (kk > 1e-9)
                {
                    const float u0 = mFft_uc[idx][0];
                    const float v0 = mFft_vc[idx][0];
                    const float u1 = mFft_uc[idx][1];
                    const float v1 = mFft_vc[idx][1];

                    // Note: Mass conserving velocity corresponds to vectors in
                    // frequency domain that are perpendicular to the wavenumber
                    // Therefore, projecting to the mass conserving vector removes divergent flow
                    const float wxx = kx * kx / kk;
                    const float wxy = ky * kx / kk;
                    const float wyy = ky * ky / kk;

                    const float f = std::exp(-kk * DT * viscosity); // viscosity

                    // update the Fourier values
                    mFft_uc[idx][0] = f * ((1 - wxx) * u0 - wxy * v0);
                    mFft_uc[idx][1] = f * ((1 - wxx) * u1 - wxy * v1);
                    mFft_vc[idx][0] = f * ((1 - wyy) * v0 - wxy * u0);
                    mFft_vc[idx][1] = f * ((1 - wyy) * v1 - wxy * u1);
                }
                idx++;
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
    CellType interpolate(XYPair& point, const auto& q)
    {
        auto clip = [](float& a, const float mn, const float mx) {
            a = std::min(mx, std::max(mn, a));
        };

        clip(point.x, 0.5, GRID_SIZE-1.5);
        clip(point.y, 0.5, GRID_SIZE-1.5);

        const int intX = static_cast<int>(point.x);
        const int intY = static_cast<int>(point.y);
        const float decX = point.x - intX;
        const float decY = point.y - intY;

        return q[POS(intX, intY)] * (1.0f - decX) * (1.0f - decY) +
               q[POS(intX, intY+1)] * (1.0f - decX) * decY +
               q[POS(intX+1, intY)] * decX * (1.0f - decY) +
               q[POS(intX+1, intY+1)] * decX * decY;
    }

    GridCellsType& mGridCells;

    fftwf_plan m_plan_u_rc, m_plan_u_cr, m_plan_v_rc, m_plan_v_cr;
    fftwf_complex* mFft_uc;
    fftwf_complex* mFft_vc;
    float* mFft_ur;
    float* mFft_vr;
};
