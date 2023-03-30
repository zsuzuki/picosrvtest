//
// copyright 2021 wave.suzuki.z@gmail.com
//
#pragma once

#include <cstdint>
#include <memory>

namespace Katakori
{
    /// 1文字
    struct FontBitmap
    {
        const uint8_t *data;
        uint32_t code;
        uint8_t width;
        uint8_t height;
        uint8_t pitch;
        int8_t offsetX;
        int8_t offsetY;
        uint8_t byteStep;

        uint64_t GetMask() const
        {
            return 1ULL << (uint64_t)(byteStep * 8 - 1);
        }
        uint64_t GetLine(const uint8_t *&ptr) const
        {
            uint64_t line = 0;
            int step = byteStep;
            for (int i = step - 1; i >= 0; i--)
            {
                line <<= 8ULL;
                line |= (uint64_t)ptr[i];
            }
            ptr += step;
            return line;
        }
    };

    /// 1フォントデータ管理クラス
    class Font
    {
        struct Impl;
        std::unique_ptr<Impl> impl;

        static void convCode(const char *&ptr, uint32_t &code);
        void getFont(uint32_t code, FontBitmap &bitmap) const;

    public:
        Font();
        ~Font();

        bool LoadFile(const char *fname);
        bool AssignMappedMemory(const void *p);

        int GetFontSize() const;
        int GetNumberOfFonts() const;
        template <typename F = void(const FontBitmap &)>
        void GetString(const char *str, F func) const
        {
            for (;;)
            {
                uint32_t code;
                convCode(str, code);
                if (code == '\0')
                    break;
                FontBitmap bitmap;
                getFont(code, bitmap);
                func(bitmap);
            }
        }
    };
}
