#ifndef LUIGI_FONT_H_
#define LUIGI_FONT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_painter.h"
#include <stdint.h>


#ifdef UI_WINDOWS
# undef _UNICODE
# undef UNICODE
#endif

#ifdef UI_FREETYPE
# include <ft2build.h>
# include FT_FREETYPE_H
# include <freetype/ftbitmap.h>
#endif


typedef struct UIFont {
    int glyphWidth, glyphHeight;
#ifdef UI_FREETYPE
    bool    isFreeType;
    FT_Face font;
# ifdef UI_UNICODE
    FT_Bitmap *glyphs;
    bool      *glyphsRendered;
    int       *glyphOffsetsX, *glyphOffsetsY, *glyphAdvance;
# else
    FT_Bitmap glyphs[128];
    bool      glyphsRendered[128];
    int       glyphOffsetsX[128], glyphOffsetsY[128];
# endif
#endif
} UIFont;


UIFont *UIFontCreate(const char *cPath, uint32_t size);
UIFont *UIFontActivate(UIFont *font);
void    UIDrawGlyph(UIPainter *painter, int x0, int y0, int c, uint32_t color);
void    UIFontDestroy(UIFont *font);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_FONT_H_
