//
// copyright 2021 wave.suzuki.z@gmail.com
//
#include <cstdio>
#include <cstdlib>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
extern "C"
{
#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"

    extern LCD_DIS sLCD_DIS;
}
#include "draw.h"
#include "font.h"

namespace Katakori
{
    // 水平線分
    void DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t col)
    {
        if (w == 0)
            return;

        uint16_t x_r = std::min(sLCD_DIS.LCD_Dis_Column, (uint16_t)(x + w));
        LCD_SetWindow(x, y, x_r, y);
        LCD_SetColor(col, x_r - x, 1);
    }
    // 垂直線分
    void DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t col)
    {
        if (y == 0)
            return;

        uint16_t y_b = std::min(sLCD_DIS.LCD_Dis_Page, (uint16_t)(y + h));
        LCD_SetWindow(x, y, x + 1, y_b);
        LCD_SetColor(col, 1, y_b - y);
    }
    // 矩形
    void DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col)
    {
        if (x0 > x1)
            std::swap(x0, x1);
        if (y0 > y1)
            std::swap(y0, y1);
        if (x0 > sLCD_DIS.LCD_Dis_Column || y0 > sLCD_DIS.LCD_Dis_Page)
            return;
        uint16_t width = std::min(x1, sLCD_DIS.LCD_Dis_Column) - x0;
        DrawHLine(x0, y0, width, col);
        if (y1 <= sLCD_DIS.LCD_Dis_Page)
            DrawHLine(x0, y1, width, col);
        uint16_t height = std::min(y1, sLCD_DIS.LCD_Dis_Page) - y0;
        DrawVLine(x0, y0, height, col);
        if (x1 <= sLCD_DIS.LCD_Dis_Column)
            DrawVLine(x1, y0, height, col);
    }
    // 塗りつぶし
    void FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col)
    {
        if (x0 > x1)
            std::swap(x0, x1);
        if (y0 > y1)
            std::swap(y0, y1);
        if (x0 > sLCD_DIS.LCD_Dis_Column || y0 > sLCD_DIS.LCD_Dis_Page)
            return;
        x1 = std::min(x1, sLCD_DIS.LCD_Dis_Column);
        y1 = std::min(y1, sLCD_DIS.LCD_Dis_Page);
        LCD_SetWindow(x0, y0, x1, y1);
        LCD_SetColor(col, x1 - x0, y1 - y0);
    }
    // 点描画
    void DrawPixel(uint16_t x, uint16_t y, uint16_t col)
    {
        LCD_SetWindow(x, y, x, y);
        LCD_SetColor(col, 1, 1);
    }
    /// 線分描画(クリッピング無し)
    void DrawLineInScreen(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col)
    {
        int16_t lx = x1 - x0;
        int16_t ly = y1 - y0;
        bool ydir = ly >= 0;
        ly = std::abs(ly);
        uint16_t dx = x0;
        uint16_t dy = y0;
        if (lx >= ly)
        {
            uint32_t y = y0 << 16;
            int32_t step = (ly << 16) / lx;
            if (!ydir)
                step = -step;
            uint16_t width = 0;
            for (uint16_t x = 0; x <= lx; x++)
            {
                uint32_t yy = y >> 16;
                if (dy != yy)
                {
                    DrawHLine(dx, dy, width, col);
                    dx = x0 + x;
                    dy = yy;
                    width = 1;
                }
                else
                    width++;
                y += step;
            }
            DrawHLine(dx, dy, width, col);
        }
        else
        {
            uint32_t x = x0 << 16;
            int32_t step = (lx << 16) / ly;
            int16_t yup = ydir ? 1 : -1;
            uint16_t height = 0;
            for (uint16_t y = 0; y <= ly; y++)
            {
                uint32_t xx = x >> 16;
                if (dx != xx)
                {
                    DrawVLine(dx, dy, height, col);
                    dx = xx;
                    dy = y0;
                    height = 1;
                }
                else
                    height++;
                // DrawPixel(xx, y0, col);
                x += step;
                y0 += yup;
            }
            DrawVLine(dx, dy, height, col);
        }
    }
    // 線分描画
    void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col)
    {
        if (x0 >= sLCD_DIS.LCD_Dis_Column || y0 >= sLCD_DIS.LCD_Dis_Page)
            return;
        if (x1 >= sLCD_DIS.LCD_Dis_Column || y1 >= sLCD_DIS.LCD_Dis_Page)
            return;
        if (x0 > x1)
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        DrawLineInScreen(x0, y0, x1, y1, col);
    }
    // 文字列描画
    void DrawString(uint16_t x, uint16_t y, uint16_t col, const char *str, const Font *font)
    {
        if (str == nullptr || font == nullptr || font->GetFontSize() > 64)
            return;

        font->GetString(str, [&](const FontBitmap &bitmap)
                        {
                            if (bitmap.data == nullptr)
                                return;
                            int width = bitmap.width;
                            int height = bitmap.height;
                            int dx = x + bitmap.offsetX;
                            int dy = y + bitmap.offsetY;
                            x += bitmap.pitch;

                            auto *fontdata = bitmap.data;
                            for (int ly = 0; ly < height; ly++)
                            {
                                uint64_t line = bitmap.GetLine(fontdata);
                                uint64_t mask = bitmap.GetMask();
                                for (int lx = 0; lx < width; lx++)
                                {
                                    if (line & mask)
                                        DrawPixel(dx + lx, dy + ly, col);
                                    line <<= 1UL;
                                }
                            }
                        });
    }

}
