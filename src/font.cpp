//
// copyright 2021 wave.suzuki.z@gmail.com
//
#include "font.h"
#include <cassert>

namespace Katakori
{
    namespace
    {
        constexpr uint32_t MAGIC = 0x544e4f46; // "FONT"
        /// ファイルヘッダ
        struct Header
        {
            uint32_t magic = 0;
            uint16_t num = 0; // 収録文字数
            uint8_t size = 0; // フォントサイズ
            uint8_t reserve = 0;
        };

        /// フォントデータ
        struct Hint
        {
            uint32_t dataOffset;
            uint16_t code;
            uint8_t width;
            uint8_t height;
            uint8_t pitch;
            int8_t offsetX;
            int8_t offsetY;
            uint8_t byteStep;
        };

        /// UTF-8からISO10646に変換
        void
        convcode(const char *&ptr, uint32_t &code)
        {
            uint32_t c0 = *ptr;
            if (c0 <= 0x7f)
            {
                // ascii
                code = c0;
                ptr++;
                return;
            }

            switch (c0 & 0xf0)
            {
            case 0xc0:
            case 0xd0:
                code = (c0 & 0x1f) << 6 | (ptr[1] & 0x3f);
                ptr += 2;
                break;
            case 0xe0:
                code = (c0 & 0xf) << 12 | (ptr[1] & 0x3f) << 6 | (ptr[2] & 0x3f);
                ptr += 3;
                break;
            default:
                break;
            }
        }

        // 指定コードのフォントを探す
        const Hint *search(uint16_t code, const Hint *hint, int num)
        {
            for (int m = 0, n = num; m <= n;)
            {
                int i = (m + n) >> 1;
                auto *h = &hint[i];
                if (h->code == code)
                    return h;
                if (code < h->code)
                    n = i - 1;
                else
                    m = i + 1;
            }
            return nullptr;
        }

    }

    struct Font::Impl
    {
        const Header *header = nullptr;
        const Hint *hint = nullptr;
        const uint8_t *bitmap = nullptr;
    };

    //
    //
    Font::Font() : impl(std::make_unique<Impl>()) {}

    //
    //
    Font::~Font() = default;

    //
    //
    void Font::convCode(const char *&ptr, uint32_t &code) { convcode(ptr, code); }

    //
    //
    void Font::getFont(uint32_t code, FontBitmap &bitmap) const
    {
        if (auto hint = search(code, impl->hint, impl->header->num))
        {
            bitmap.data = impl->bitmap + hint->dataOffset;
            bitmap.code = hint->code;
            bitmap.width = hint->width;
            bitmap.height = hint->height;
            bitmap.offsetX = hint->offsetX;
            bitmap.offsetY = hint->offsetY;
            bitmap.pitch = hint->pitch;
            bitmap.byteStep = hint->byteStep;
        }
        else
        {
            bitmap.data = nullptr;
        }
    }

    //
    //
    bool Font::LoadFile(const char *fname) { return false; }

    //
    //
    bool Font::AssignMappedMemory(const void *p)
    {
        impl->header = reinterpret_cast<const Header *>(p);
        if (impl->header->magic == MAGIC)
        {
            impl->bitmap = reinterpret_cast<const uint8_t *>(p);
            impl->hint = reinterpret_cast<const Hint *>(impl->header + 1);
        }
        else
        {
            impl->header = nullptr;
            impl->bitmap = nullptr;
            impl->hint = nullptr;
            return false;
        }
        return true;
    }

    //
    //
    int Font::GetFontSize() const
    {
        assert(impl->header);
        return impl->header->size;
    }

    //
    //
    int Font::GetNumberOfFonts() const
    {
        assert(impl->header);
        return impl->header->num;
    }
}
