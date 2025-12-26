#ifndef LUIGI_CODE_H_
#define LUIGI_CODE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "font.h"
#include "timing.h"
#include "ui_element.h"
#include "ui_painter.h"
#include "ui_rect.h"
#include "ui_scroll.h"
#include "ui_string.h"
#include <stdint.h>


#define UI_CODE_NO_MARGIN  (1 << 0)
#define UI_CODE_SELECTABLE (1 << 1)

#define UI_SIZE_CODE_MARGIN     (ui.activeFont->glyphWidth * 5)
#define UI_SIZE_CODE_MARGIN_GAP (ui.activeFont->glyphWidth * 1)


//


#define _UICharIsDigit(c)                    (c >= '0' && c <= '9')
#define _UICharIsAlpha(c)                    ((('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || c > 127))
#define _UICharIsAlphaOrDigitOrUnderscore(c) (_UICharIsAlpha(c) || _UICharIsDigit(c) || c == '_')


//


typedef struct UICodeDecorateLine {
    UIRectangle bounds;
    int         index; // Starting at 1!
    int         x, y;  // Position where additional text can be drawn.
    UIPainter  *painter;
} UICodeDecorateLine;


typedef struct UICodeLine {
    int offset, bytes;
} UICodeLine;


typedef struct UICode {
    UIElement    e;
    UIScrollBar *vScroll, *hScroll;
    UICodeLine  *lines;
    UIFont      *font;
    int          lineCount, focused;
    bool         moveScrollToFocusNextLayout;
    bool         leftDownInMargin;
    char        *content;
    size_t       contentBytes;
    int          tabSize;
    int          columns;
    UI_CLOCK_T   lastAnimateTime;
    struct {
        int line, offset;
    } selection[4 /* start, end, anchor, caret */];
    int  verticalMotionColumn;
    bool useVerticalMotionColumn;
    bool moveScrollToCaretNextLayout;
} UICode;


//

UICode *UICodeCreate(UIElement *parent, uint32_t flags);
void    UICodePositionToByte(UICode *code, int x, int y, int *line, int *byte);
int     UICodeHitTest(UICode *code, int x, int y);
void    UICodeMoveCaret(UICode *code, bool backward, bool word);
void    UICodeFocusLine(UICode *code, int index);
void    UICodeInsertContent(UICode *code, const char *content, ptrdiff_t byteCount, bool replace);

int UIDrawStringHighlighted(UIPainter *painter, UIRectangle lineBounds, const char *string,
                            ptrdiff_t bytes, int tabSize, UIStringSelection *selection);


int  _UICodeByteToColumn(UICode *code, int line, int byte);
int  _UICodeColumnToByte(UICode *code, int line, int column);
void _UICodeUpdateSelection(UICode *code);
void _UICodeSetVerticalMotionColumn(UICode *code, bool restore);
void _UICodeCopyText(void *cp);
int  _UICodeMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_CODE_H_
