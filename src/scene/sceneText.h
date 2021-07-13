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
    auto POS(auto x, auto y) { return GridCellsType::POS(x,y); }
    static constexpr uint16_t GRID_SIZE = GridCellsType::GRID_SIZE;
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
        constexpr uint16_t bufWidth{GRID_SIZE};
        constexpr uint16_t bufHeight{GRID_SIZE};
        uint8_t buf[bufWidth * bufHeight];
        memset(buf, 0, bufWidth * bufHeight * sizeof(uint8_t));
        uint16_t x{}, fontSize{45};

        std::time_t ct = std::time(0);
        char mbstr[100];
        uint16_t maxHeight{};
        if (std::strftime(mbstr, sizeof(mbstr), "%H:%M:%S", std::localtime(&ct))) {
            for (uint16_t i=0; i<strlen(mbstr); ++i) {
                auto [newX, thisHeight] = copyCharacterToBuffer(mbstr[i], fontSize, x, 0, buf, bufWidth, bufHeight);
                x = newX;
                maxHeight = std::max(maxHeight, thisHeight);
            }
        }

        uint16_t gridX = (GRID_SIZE - x)/2;
        uint16_t gridY = (GRID_SIZE - maxHeight)/2;

        for(unsigned int j=0; j < maxHeight; ++j) {
            for(unsigned int i=0; i < x; ++i) {
                uint16_t inIdx = i + bufWidth * j;
                uint8_t val = buf[inIdx];
                if (val > 0) {
                    uint16_t outIdx = POS(i+gridX, j+gridY);
                    auto [den, vel] = baseType::getFireSource();
                    baseType::mGridCells.density[outIdx] += den * 0.05;
                    baseType::mGridCells.velocity[outIdx] += vel * 0.05;
                }
            }
        }

        constexpr float velWgt = 0.01f;
        constexpr int size = std::max(1, GRID_SIZE/5);
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
    std::pair<uint16_t, uint16_t> copyCharacterToBuffer(const char c, const uint16_t fontHeight, uint16_t x, uint16_t y,
                                  uint8_t* pBuf, const uint16_t width, const uint16_t height)
    {
        FT_Set_Pixel_Sizes(face, 0, fontHeight);
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            throw std::runtime_error("FREETYPE: Failed to load glyph");
        }

        // adjust the render origin
        x += face->glyph->bitmap_left;
        y = std::max(0, y - face->glyph->bitmap_top);

        if ((face->glyph->bitmap.width + x >= width) ||
            (face->glyph->bitmap.rows + y >= height)) {
            return std::pair(0,0);
        }

        uint8_t* pBuffer = face->glyph->bitmap.buffer;
        for(unsigned int j=0; j < face->glyph->bitmap.rows; ++j) {
            for(unsigned int i=0; i < face->glyph->bitmap.width; ++i) {
                uint16_t outIdx = i+x + width * (j+y);
                pBuf[outIdx] = *pBuffer;
                pBuffer++;
            }
        }
        return std::pair{x + face->glyph->advance.x / 64,
                y + face->glyph->bitmap.rows};
    }

    FT_Face face;
};

