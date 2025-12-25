#include "font.h"
#include "ui.h"
#include "ui_painter.h"
#include "ui_rect.h"
#include "utils.h"
#include <stdint.h>


// Taken from https://commons.wikimedia.org/wiki/File:Codepage-437.png
// Public domain.

const uint64_t _uiFont[] = {
    0x0000000000000000UL, 0x0000000000000000UL, 0xBD8181A5817E0000UL, 0x000000007E818199UL,
    0xC3FFFFDBFF7E0000UL, 0x000000007EFFFFE7UL, 0x7F7F7F3600000000UL, 0x00000000081C3E7FUL,
    0x7F3E1C0800000000UL, 0x0000000000081C3EUL, 0xE7E73C3C18000000UL, 0x000000003C1818E7UL,
    0xFFFF7E3C18000000UL, 0x000000003C18187EUL, 0x3C18000000000000UL, 0x000000000000183CUL,
    0xC3E7FFFFFFFFFFFFUL, 0xFFFFFFFFFFFFE7C3UL, 0x42663C0000000000UL, 0x00000000003C6642UL,
    0xBD99C3FFFFFFFFFFUL, 0xFFFFFFFFFFC399BDUL, 0x331E4C5870780000UL, 0x000000001E333333UL,
    0x3C666666663C0000UL, 0x0000000018187E18UL, 0x0C0C0CFCCCFC0000UL, 0x00000000070F0E0CUL,
    0xC6C6C6FEC6FE0000UL, 0x0000000367E7E6C6UL, 0xE73CDB1818000000UL, 0x000000001818DB3CUL,
    0x1F7F1F0F07030100UL, 0x000000000103070FUL, 0x7C7F7C7870604000UL, 0x0000000040607078UL,
    0x1818187E3C180000UL, 0x0000000000183C7EUL, 0x6666666666660000UL, 0x0000000066660066UL,
    0xD8DEDBDBDBFE0000UL, 0x00000000D8D8D8D8UL, 0x6363361C06633E00UL, 0x0000003E63301C36UL,
    0x0000000000000000UL, 0x000000007F7F7F7FUL, 0x1818187E3C180000UL, 0x000000007E183C7EUL,
    0x1818187E3C180000UL, 0x0000000018181818UL, 0x1818181818180000UL, 0x00000000183C7E18UL,
    0x7F30180000000000UL, 0x0000000000001830UL, 0x7F060C0000000000UL, 0x0000000000000C06UL,
    0x0303000000000000UL, 0x0000000000007F03UL, 0xFF66240000000000UL, 0x0000000000002466UL,
    0x3E1C1C0800000000UL, 0x00000000007F7F3EUL, 0x3E3E7F7F00000000UL, 0x0000000000081C1CUL,
    0x0000000000000000UL, 0x0000000000000000UL, 0x18183C3C3C180000UL, 0x0000000018180018UL,
    0x0000002466666600UL, 0x0000000000000000UL, 0x36367F3636000000UL, 0x0000000036367F36UL,
    0x603E0343633E1818UL, 0x000018183E636160UL, 0x1830634300000000UL, 0x000000006163060CUL,
    0x3B6E1C36361C0000UL, 0x000000006E333333UL, 0x000000060C0C0C00UL, 0x0000000000000000UL,
    0x0C0C0C0C18300000UL, 0x0000000030180C0CUL, 0x30303030180C0000UL, 0x000000000C183030UL,
    0xFF3C660000000000UL, 0x000000000000663CUL, 0x7E18180000000000UL, 0x0000000000001818UL,
    0x0000000000000000UL, 0x0000000C18181800UL, 0x7F00000000000000UL, 0x0000000000000000UL,
    0x0000000000000000UL, 0x0000000018180000UL, 0x1830604000000000UL, 0x000000000103060CUL,
    0xDBDBC3C3663C0000UL, 0x000000003C66C3C3UL, 0x1818181E1C180000UL, 0x000000007E181818UL,
    0x0C183060633E0000UL, 0x000000007F630306UL, 0x603C6060633E0000UL, 0x000000003E636060UL,
    0x7F33363C38300000UL, 0x0000000078303030UL, 0x603F0303037F0000UL, 0x000000003E636060UL,
    0x633F0303061C0000UL, 0x000000003E636363UL, 0x18306060637F0000UL, 0x000000000C0C0C0CUL,
    0x633E6363633E0000UL, 0x000000003E636363UL, 0x607E6363633E0000UL, 0x000000001E306060UL,
    0x0000181800000000UL, 0x0000000000181800UL, 0x0000181800000000UL, 0x000000000C181800UL,
    0x060C183060000000UL, 0x000000006030180CUL, 0x00007E0000000000UL, 0x000000000000007EUL,
    0x6030180C06000000UL, 0x00000000060C1830UL, 0x18183063633E0000UL, 0x0000000018180018UL,
    0x7B7B63633E000000UL, 0x000000003E033B7BUL, 0x7F6363361C080000UL, 0x0000000063636363UL,
    0x663E6666663F0000UL, 0x000000003F666666UL, 0x03030343663C0000UL, 0x000000003C664303UL,
    0x66666666361F0000UL, 0x000000001F366666UL, 0x161E1646667F0000UL, 0x000000007F664606UL,
    0x161E1646667F0000UL, 0x000000000F060606UL, 0x7B030343663C0000UL, 0x000000005C666363UL,
    0x637F636363630000UL, 0x0000000063636363UL, 0x18181818183C0000UL, 0x000000003C181818UL,
    0x3030303030780000UL, 0x000000001E333333UL, 0x1E1E366666670000UL, 0x0000000067666636UL,
    0x06060606060F0000UL, 0x000000007F664606UL, 0xC3DBFFFFE7C30000UL, 0x00000000C3C3C3C3UL,
    0x737B7F6F67630000UL, 0x0000000063636363UL, 0x63636363633E0000UL, 0x000000003E636363UL,
    0x063E6666663F0000UL, 0x000000000F060606UL, 0x63636363633E0000UL, 0x000070303E7B6B63UL,
    0x363E6666663F0000UL, 0x0000000067666666UL, 0x301C0663633E0000UL, 0x000000003E636360UL,
    0x18181899DBFF0000UL, 0x000000003C181818UL, 0x6363636363630000UL, 0x000000003E636363UL,
    0xC3C3C3C3C3C30000UL, 0x00000000183C66C3UL, 0xDBC3C3C3C3C30000UL, 0x000000006666FFDBUL,
    0x18183C66C3C30000UL, 0x00000000C3C3663CUL, 0x183C66C3C3C30000UL, 0x000000003C181818UL,
    0x0C183061C3FF0000UL, 0x00000000FFC38306UL, 0x0C0C0C0C0C3C0000UL, 0x000000003C0C0C0CUL,
    0x1C0E070301000000UL, 0x0000000040607038UL, 0x30303030303C0000UL, 0x000000003C303030UL,
    0x0000000063361C08UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000FF0000000000UL,
    0x0000000000180C0CUL, 0x0000000000000000UL, 0x3E301E0000000000UL, 0x000000006E333333UL,
    0x66361E0606070000UL, 0x000000003E666666UL, 0x03633E0000000000UL, 0x000000003E630303UL,
    0x33363C3030380000UL, 0x000000006E333333UL, 0x7F633E0000000000UL, 0x000000003E630303UL,
    0x060F0626361C0000UL, 0x000000000F060606UL, 0x33336E0000000000UL, 0x001E33303E333333UL,
    0x666E360606070000UL, 0x0000000067666666UL, 0x18181C0018180000UL, 0x000000003C181818UL,
    0x6060700060600000UL, 0x003C666660606060UL, 0x1E36660606070000UL, 0x000000006766361EUL,
    0x18181818181C0000UL, 0x000000003C181818UL, 0xDBFF670000000000UL, 0x00000000DBDBDBDBUL,
    0x66663B0000000000UL, 0x0000000066666666UL, 0x63633E0000000000UL, 0x000000003E636363UL,
    0x66663B0000000000UL, 0x000F06063E666666UL, 0x33336E0000000000UL, 0x007830303E333333UL,
    0x666E3B0000000000UL, 0x000000000F060606UL, 0x06633E0000000000UL, 0x000000003E63301CUL,
    0x0C0C3F0C0C080000UL, 0x00000000386C0C0CUL, 0x3333330000000000UL, 0x000000006E333333UL,
    0xC3C3C30000000000UL, 0x00000000183C66C3UL, 0xC3C3C30000000000UL, 0x0000000066FFDBDBUL,
    0x3C66C30000000000UL, 0x00000000C3663C18UL, 0x6363630000000000UL, 0x001F30607E636363UL,
    0x18337F0000000000UL, 0x000000007F63060CUL, 0x180E181818700000UL, 0x0000000070181818UL,
    0x1800181818180000UL, 0x0000000018181818UL, 0x18701818180E0000UL, 0x000000000E181818UL,
    0x000000003B6E0000UL, 0x0000000000000000UL, 0x63361C0800000000UL, 0x00000000007F6363UL,
};


#ifdef UI_FREETYPE
void UIEnsureGlyphRendered(UIFont *font, int c)
{
    if (!font->glyphsRendered[c]) {
        FT_Load_Char(font->font,
                     c == 24   ? 0x2191
                     : c == 25 ? 0x2193
                     : c == 26 ? 0x2192
                     : c == 27 ? 0x2190
                               : c,
                     FT_LOAD_DEFAULT);
# ifdef UI_FREETYPE_SUBPIXEL
        FT_Render_Glyph(font->font->glyph, FT_RENDER_MODE_LCD);
# else
        FT_Render_Glyph(font->font->glyph, FT_RENDER_MODE_NORMAL);
# endif
        FT_Bitmap_Copy(ui.ft, &font->font->glyph->bitmap, &font->glyphs[c]);
        font->glyphOffsetsX[c] = font->font->glyph->bitmap_left;
        font->glyphOffsetsY[c] =
            font->font->size->metrics.ascender / 64 - font->font->glyph->bitmap_top;
        font->glyphAdvance[c]   = font->font->glyph->advance.x / 64;
        font->glyphsRendered[c] = true;
    }
}
#endif

void UIDrawGlyph(UIPainter *painter, int x0, int y0, int c, uint32_t color)
{
#ifdef UI_FREETYPE
    UIFont *font = ui.activeFont;

    float color0 = ((color >> 16) & 0xFF) / 255.0f;
    float color1 = ((color >> 8) & 0xFF) / 255.0f;
    float color2 = ((color >> 0) & 0xFF) / 255.0f;

# if 0
    color0 *= color0;
    color1 *= color1;
    color2 *= color2;
# endif

    if (font->isFreeType) {
# ifdef UI_UNICODE
        if (c < 0)
            c = '?';
# else
        if (c < 0 || c > 127)
            c = '?';
# endif
        if (c == '\r')
            c = ' ';

        UIEnsureGlyphRendered(font, c);

        FT_Bitmap *bitmap = &font->glyphs[c];
        x0 += font->glyphOffsetsX[c], y0 += font->glyphOffsetsY[c];

        for (int y = 0; y < (int)bitmap->rows; y++) {
            if (y0 + y < painter->clip.t)
                continue;
            if (y0 + y >= painter->clip.b)
                break;

            int width = bitmap->pixel_mode == FT_PIXEL_MODE_LCD ? bitmap->width / 3 : bitmap->width;

            for (int x = 0; x < width; x++) {
                if (x0 + x < painter->clip.l)
                    continue;
                if (x0 + x >= painter->clip.r)
                    break;

                uint32_t *destination = painter->bits + (x0 + x) + (y0 + y) * painter->width;
                uint32_t  original    = *destination;
                float     a;

                if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
                    a = (((uint8_t *)bitmap->buffer)[(x >> 3) + y * bitmap->pitch] &
                         (0x80 >> (x & 7)))
                            ? 1
                            : 0;
                } else if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
                    a = ((uint8_t *)bitmap->buffer)[x + y * bitmap->pitch] / 255.0f;
                } else {
                    a = 0;
                }

                float original0 = ((original >> 16) & 0xFF) / 255.0f;
                float original1 = ((original >> 8) & 0xFF) / 255.0f;
                float original2 = ((original >> 0) & 0xFF) / 255.0f;

# if 0
                original0 *= original0;
                original1 *= original1;
                original2 *= original2;
# endif

                float new0 = original0 + (color0 - original0) * a;
                float new1 = original1 + (color1 - original1) * a;
                float new2 = original2 + (color2 - original2) * a;

# if 0
                new0 = sqrtf(new0);
                new1 = sqrtf(new1);
                new2 = sqrtf(new2);
# endif

                uint32_t result = 0xFF000000 | ((uint32_t)(new0 * 255.0f) << 16) |
                                  ((uint32_t)(new1 * 255.0f) << 8) |
                                  ((uint32_t)(new2 * 255.0f) << 0);
                *destination = result;
            }
        }

        return;
    }
#endif

    if (c < 0 || c > 127)
        c = '?';

    UIRectangle rectangle =
        UIRectangleIntersection(painter->clip, UI_RECT_4(x0, x0 + 8, y0, y0 + 16));

    const uint8_t *data = (const uint8_t *)_uiFont + c * 16;

    for (int i = rectangle.t; i < rectangle.b; i++) {
        uint32_t *bits = painter->bits + i * painter->width + rectangle.l;
        uint8_t   byte = data[i - y0];

        for (int j = rectangle.l; j < rectangle.r; j++) {
            if (byte & (1 << (j - x0))) {
                *bits = color;
            }

            bits++;
        }
    }
}

UIFont *UIFontCreate(const char *cPath, uint32_t size)
{
    UIFont *font = (UIFont *)UI_CALLOC(sizeof(UIFont));

#ifndef UI_FREETYPE
    (void)cPath;
#else
# ifdef UI_UNICODE
    font->glyphs         = (FT_Bitmap *)UI_CALLOC(sizeof(FT_Bitmap) * (_UNICODE_MAX_CODEPOINT + 1));
    font->glyphsRendered = (bool *)UI_CALLOC(sizeof(bool) * (_UNICODE_MAX_CODEPOINT + 1));
    font->glyphOffsetsX  = (int *)UI_CALLOC(sizeof(int) * (_UNICODE_MAX_CODEPOINT + 1));
    font->glyphOffsetsY  = (int *)UI_CALLOC(sizeof(int) * (_UNICODE_MAX_CODEPOINT + 1));
    font->glyphAdvance   = (int *)UI_CALLOC(sizeof(int) * (_UNICODE_MAX_CODEPOINT + 1));
# endif
    if (cPath) {
        int ret = FT_New_Face(ui.ft, cPath, 0, &font->font);
        if (ret == 0) {
            FT_Select_Charmap(font->font, FT_ENCODING_UNICODE);
            if (FT_HAS_FIXED_SIZES(font->font) && font->font->num_fixed_sizes) {
                // Look for the smallest strike that's at least `size`.
                int j = 0;

                for (int i = 0; i < font->font->num_fixed_sizes; i++) {
                    if ((uint32_t)font->font->available_sizes[i].height >= size &&
                        font->font->available_sizes[i].y_ppem <
                            font->font->available_sizes[j].y_ppem) {
                        j = i;
                    }
                }

                FT_Set_Pixel_Sizes(font->font, font->font->available_sizes[j].x_ppem / 64,
                                   font->font->available_sizes[j].y_ppem / 64);
            } else {
                FT_Set_Char_Size(font->font, 0, size * 64, 100, 100);
            }

            FT_Load_Char(font->font, 'a', FT_LOAD_DEFAULT);
            font->glyphWidth = font->font->glyph->advance.x / 64;
            font->glyphHeight =
                (font->font->size->metrics.ascender - font->font->size->metrics.descender) / 64;
            font->isFreeType = true;
            return font;
        } else
            printf("Cannot load font %s : %d\n", cPath, ret);
    }
#endif

    font->glyphWidth  = 9;
    font->glyphHeight = 16;
    return font;
}


UIFont *UIFontActivate(UIFont *font)
{
    UIFont *previous = ui.activeFont;
    ui.activeFont    = font;
    return previous;
}


void UIFontDestroy(UIFont *font)
{
#ifdef UI_FREETYPE
    for (uintptr_t i = 0; i < _UNICODE_MAX_CODEPOINT; i++) {
        if (font->glyphsRendered[i]) {
            FT_Bitmap_Done(ui.ft, &font->glyphs[i]);
        }
    }

    FT_Done_Face(font->font);
    UI_FREE(font->glyphs);
    UI_FREE(font->glyphsRendered);
    UI_FREE(font->glyphOffsetsX);
    UI_FREE(font->glyphOffsetsY);
    UI_FREE(font->glyphAdvance);
#endif
    UI_FREE(font);
}
