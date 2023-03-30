//
// copyright 2021 wave.suzuki.z@gmail.com
//
#pragma once

#include <cstdint>
#include <memory>

namespace Katakori
{
    class Font;

    static constexpr uint16_t ToRed(uint16_t c)
    {
        return c >> 3;
    }
    static constexpr uint16_t ToGreen(uint16_t c)
    {
        return c >> 2;
    }
    static constexpr uint16_t ToBlue(uint16_t c)
    {
        return c >> 3;
    }
    static constexpr uint16_t Color(uint16_t r, uint16_t g, uint16_t b)
    {
        return (ToRed(r) << 11) | (ToGreen(g) << 5) | ToBlue(b);
    }
    static constexpr uint16_t White = Color(255, 255, 255);
    static constexpr uint16_t Black = Color(0, 0, 0);
    static constexpr uint16_t Red = Color(255, 0, 0);
    static constexpr uint16_t Green = Color(0, 255, 0);
    static constexpr uint16_t Blue = Color(0, 0, 255);
    static constexpr uint16_t Gray = Color(128, 128, 128);
    static constexpr uint16_t Yellow = Color(255, 255, 0);
    static constexpr uint16_t Purple = Color(255, 0, 255);
    static constexpr uint16_t LightGray = Color(192, 192, 192);
    static constexpr uint16_t DarkGray = Color(64, 64, 64);

    void DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t col);
    void DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t col);
    void DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col);
    void FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col);
    void DrawPixel(uint16_t x, uint16_t y, uint16_t col);
    void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col);
    void DrawString(uint16_t x, uint16_t y, uint16_t col, const char *str, const Font *font);
}
