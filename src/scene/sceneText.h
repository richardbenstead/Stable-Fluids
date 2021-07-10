#pragma once
#include "src/scene/sceneBase.h"
#include <stdexcept>
#include <ft2build.h>
#include FT_FREETYPE_H  


template<typename GridCellsType>
class SceneText : public SceneBase
{
public:
    SceneText(GridCellsType& gc) : mGridCells(gc)
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
                                                           10, // gravity
                                                           0.95, // density
                                                           0.0f};} // diffusion

    void update([[maybe_unused]] const float time)
    {
        constexpr uint8_t bufWidth{200};
        constexpr uint8_t bufHeight{80};
        uint8_t buf[bufWidth * bufHeight];
        memset(buf, 0, bufWidth * bufHeight * sizeof(uint8_t));
        uint8_t x = 0;
        uint8_t y = 10;
        uint8_t fontSize = 20;

        x += copyCharacterToBuffer('0', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer('8', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer(':', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer('1', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer('1', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer(':', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer('1', fontSize, x, y, buf, bufWidth, bufHeight);
        x += copyCharacterToBuffer('9', fontSize, x, y, buf, bufWidth, bufHeight);

        uint8_t gridX = 10;
        uint8_t gridY = 10;

        for(unsigned int j=0; j < bufHeight; ++j) {
            for(unsigned int i=0; i < x; ++i) {
                uint16_t inIdx = i + bufWidth * j;
                uint8_t val = buf[inIdx];
                uint16_t outIdx = POS(i+gridX, j+gridY);
                mGridCells.density[outIdx].r += val/2000.0f;
                mGridCells.density[outIdx].g += val/4000.0f;
                mGridCells.density[outIdx].b += val/4000.0f;
            }
        }
    }

private:

    uint8_t copyCharacterToBuffer(const char c, const uint16_t fontHeight, uint16_t x, uint16_t y,
                                  uint8_t* pBuf, const uint16_t width, const uint16_t height)
    {
        FT_Set_Pixel_Sizes(face, 0, fontHeight);
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            throw std::runtime_error("FREETYPE: Failed to load glyph");
        }

        x += face->glyph->bitmap_left;
        y = std::max(0, y - face->glyph->bitmap_top);

        //std::cout << face->glyph->bitmap_top
        if ((face->glyph->bitmap.width + x >= width) ||
            (face->glyph->bitmap.rows + y >= height)) {
            return 0;
        }

        uint8_t* pBuffer = face->glyph->bitmap.buffer;
        for(unsigned int j=0; j < face->glyph->bitmap.rows; ++j) {
            for(unsigned int i=0; i < face->glyph->bitmap.width; ++i) {
                uint16_t outIdx = i+x + width * (j+y);
                pBuf[outIdx] = *pBuffer;
                pBuffer++;
            }
        }
        return face->glyph->advance.x / 64 + face->glyph->bitmap_left;
    }

    GridCellsType& mGridCells;
    FT_Face face;
};

