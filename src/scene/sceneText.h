#pragma once
#include "src/scene/sceneBase.h"
#include <stdexcept>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <chrono>
#include <iostream>


template<typename GridCellsType>
class SceneText : public SceneBase<GridCellsType>
{
    using baseType = SceneBase<GridCellsType>;
    int32_t POS(const int32_t x, const int32_t y) { return GridCellsType::POS(x,y); }
    static constexpr int16_t GRID_SIZE = GridCellsType::GRID_SIZE;
public:
    SceneText(GridCellsType& gc) : baseType(gc)
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            throw std::runtime_error("FREETYPE: Could not init FreeType Library");
        }

        if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face)) {
            throw std::runtime_error("FREETYPE: Failed to load font");
        }
    }

    constexpr SceneParams getParams() { return SceneParams{0, // viscosity
                                                           -9, // gravity
                                                           0.97, // density
                                                           0.001f};} // diffusion

    void update([[maybe_unused]] const float time)
    {
        constexpr int16_t bufWidth{GRID_SIZE};
        constexpr int16_t bufHeight{GRID_SIZE};
        int8_t buf[bufWidth * bufHeight];
        memset(buf, 0, bufWidth * bufHeight * sizeof(int8_t));
        int16_t x{}, fontSize(GRID_SIZE * 0.15f);

        std::time_t ct = std::time(0);
        char mbstr[100];
        int16_t maxHeight{};
        int16_t slen(strlen(mbstr));
        if (std::strftime(mbstr, sizeof(mbstr), "%H:%M:%S", std::localtime(&ct))) {
            for (int16_t i=0; i<slen; ++i) {
                auto [newX, thisHeight] = copyCharacterToBuffer(mbstr[i], fontSize, x, 0, buf, bufWidth, bufHeight);
                x = newX;
                maxHeight = std::max(maxHeight, thisHeight);
            }
        }

        const int16_t gridX = (GRID_SIZE - x)/2;
        const int16_t gridY = (GRID_SIZE - maxHeight)/2;

        for(int16_t j=0; j < maxHeight; ++j) {
            for(int16_t i=0; i < x; ++i) {
                int16_t inIdx = i + bufWidth * j;
                int8_t val = buf[inIdx];
                if (val != 0) {
                    int32_t outIdx = POS(i+gridX, j+gridY);
                    auto [den, vel] = baseType::getFireSource();
                    baseType::mGridCells.density[outIdx] += den * static_cast<float>(GRID_SIZE) * 0.00015f;
                    baseType::mGridCells.velocity[outIdx] += vel * 0.05;
                }
            }
        }

        constexpr float velWgt = 0.01f;
        constexpr int16_t size = std::max(1, GRID_SIZE/5);
        struct SourceInfo {
            float xAmp, xOffset, xPhase, xSpeed;
            float yAmp, yOffset, yPhase, ySpeed;
            float r,g,b;
        };

        std::vector<SourceInfo> mSources;
        mSources.push_back(SourceInfo{GRID_SIZE * 0.4f, GRID_SIZE/2, 0, 10,
                                      GRID_SIZE * 0.4f, GRID_SIZE/2, 0, 13,
                                      0, 0, 0.05});

        for(auto s : mSources) {
            baseType::addGaussian(sin(time * s.xSpeed + s.xPhase) * s.xAmp + s.xOffset,
                                  sin(time * s.ySpeed + s.yPhase) * s.yAmp + s.yOffset,
                                  size, size, s.r, s.g, s.b,
                                  velWgt * cos(time * s.xSpeed + s.xPhase) * s.xAmp,
                                  velWgt * cos(time * s.ySpeed + s.yPhase) * s.yAmp);
        }
    }

private:
    std::pair<int16_t, int16_t> copyCharacterToBuffer(const char c, const int16_t fontHeight, int16_t x, int16_t y,
                                  int8_t* pBuf, const int16_t width, const int16_t height)
    {
        FT_Set_Pixel_Sizes(face, 0, fontHeight);
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            throw std::runtime_error("FREETYPE: Failed to load glyph");
        }

        // adjust the render origin
        x += face->glyph->bitmap_left;
        y = std::max(0, y - face->glyph->bitmap_top);

        if ((face->glyph->bitmap.width + x >= static_cast<uint16_t>(width)) ||
            (face->glyph->bitmap.rows + y >= static_cast<uint16_t>(height))) {
            return std::pair(0,0);
        }

        uint8_t* pBuffer = face->glyph->bitmap.buffer;
        for(int16_t j=0; j < static_cast<int16_t>(face->glyph->bitmap.rows); ++j) {
            for(int16_t i=0; i < static_cast<int16_t>(face->glyph->bitmap.width); ++i) {
                int16_t outIdx = i+x + width * (j+y);
                pBuf[outIdx] = *pBuffer;
                pBuffer++;
            }
        }
        return std::pair{x + face->glyph->advance.x / 64,
                y + face->glyph->bitmap.rows};
    }

    FT_Face face;
};

