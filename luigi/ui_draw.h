#ifndef LUIGI_DRAW_H_
#define LUIGI_DRAW_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_painter.h"
#include "ui_string.h"


#ifndef UI_DRAW_CONTROL_CUSTOM
# define UIDrawControl UIDrawControlDefault
#endif
#define UI_DRAW_CONTROL_PUSH_BUTTON         (1)
#define UI_DRAW_CONTROL_DROP_DOWN           (2)
#define UI_DRAW_CONTROL_MENU_ITEM           (3)
#define UI_DRAW_CONTROL_CHECKBOX            (4)
#define UI_DRAW_CONTROL_LABEL               (5)
#define UI_DRAW_CONTROL_SPLITTER            (6)
#define UI_DRAW_CONTROL_SCROLL_TRACK        (7)
#define UI_DRAW_CONTROL_SCROLL_UP           (8)
#define UI_DRAW_CONTROL_SCROLL_DOWN         (9)
#define UI_DRAW_CONTROL_SCROLL_THUMB        (10)
#define UI_DRAW_CONTROL_GAUGE               (11)
#define UI_DRAW_CONTROL_SLIDER              (12)
#define UI_DRAW_CONTROL_TEXTBOX             (13)
#define UI_DRAW_CONTROL_MODAL_POPUP         (14)
#define UI_DRAW_CONTROL_MENU                (15)
#define UI_DRAW_CONTROL_TABLE_ROW           (16)
#define UI_DRAW_CONTROL_TABLE_CELL          (17)
#define UI_DRAW_CONTROL_TABLE_BACKGROUND    (18)
#define UI_DRAW_CONTROL_TABLE_HEADER        (19)
#define UI_DRAW_CONTROL_MDI_CHILD           (20)
#define UI_DRAW_CONTROL_TAB                 (21)
#define UI_DRAW_CONTROL_TAB_BAND            (22)
#define UI_DRAW_CONTROL_TYPE_MASK           (0xFF)
#define UI_DRAW_CONTROL_STATE_SELECTED      (1 << 24)
#define UI_DRAW_CONTROL_STATE_VERTICAL      (1 << 25)
#define UI_DRAW_CONTROL_STATE_INDETERMINATE (1 << 26)
#define UI_DRAW_CONTROL_STATE_CHECKED       (1 << 27)
#define UI_DRAW_CONTROL_STATE_HOVERED       (1 << 28)
#define UI_DRAW_CONTROL_STATE_FOCUSED       (1 << 29)
#define UI_DRAW_CONTROL_STATE_PRESSED       (1 << 30)
#define UI_DRAW_CONTROL_STATE_DISABLED      (1 << 31)
#define UI_DRAW_CONTROL_STATE_FROM_ELEMENT(x)                                                      \
    ((((x)->flags & UI_ELEMENT_DISABLED) ? UI_DRAW_CONTROL_STATE_DISABLED : 0) |                   \
     (((x)->window->hovered == (x)) ? UI_DRAW_CONTROL_STATE_HOVERED : 0) |                         \
     (((x)->window->focused == (x)) ? UI_DRAW_CONTROL_STATE_FOCUSED : 0) |                         \
     (((x)->window->pressed == (x)) ? UI_DRAW_CONTROL_STATE_PRESSED : 0))


//


bool UIDrawLine(UIPainter *painter, int x0, int y0, int x1, int y1, uint32_t color);
void UIDrawCircle(UIPainter *painter, int cx, int cy, int radius, uint32_t fillColor,
                  uint32_t outlineColor, bool hollow);
void UIDrawTriangle(UIPainter *painter, int x0, int y0, int x1, int y1, int x2, int y2,
                    uint32_t color);
void UIDrawTriangleOutline(UIPainter *painter, int x0, int y0, int x1, int y1, int x2, int y2,
                           uint32_t color);
void UIDrawBlock(UIPainter *painter, UIRectangle rectangle, uint32_t color);
void UIDrawString(UIPainter *painter, UIRectangle r, const char *string, ptrdiff_t bytes,
                  uint32_t color, int align, UIStringSelection *selection);
void UIDrawBorder(UIPainter *painter, UIRectangle r, uint32_t borderColor, UIRectangle borderSize);
void UIDrawRectangle(UIPainter *painter, UIRectangle r, uint32_t mainColor, uint32_t borderColor,
                     UIRectangle borderSize);


void UIDrawControlDefault(UIPainter *painter, UIRectangle bounds, uint32_t mode, const char *label,
                          ptrdiff_t labelBytes, double position, float scale);

void UIDrawInvert(UIPainter *painter, UIRectangle rectangle);

#ifdef __cplusplus
}
#endif

#endif // LUIGI_DRAW_H_
