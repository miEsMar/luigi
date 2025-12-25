#include "ui_draw.h"
#include "font.h"
#include "ui.h"
#include "ui_checkbox.h"
#include "ui_string.h"
#include "utils.h"


#ifdef UI_SSE2
# include <xmmintrin.h>
#endif


//


void UIDrawBlock(UIPainter *painter, UIRectangle rectangle, uint32_t color)
{
    rectangle = UIRectangleIntersection(painter->clip, rectangle);

    if (!UI_RECT_VALID(rectangle)) {
        return;
    }

#ifdef UI_SSE2
    __m128i color4 = _mm_set_epi32(color, color, color, color);
#endif

    for (int line = rectangle.t; line < rectangle.b; line++) {
        uint32_t *bits  = painter->bits + line * painter->width + rectangle.l;
        int       count = UI_RECT_WIDTH(rectangle);

#ifdef UI_SSE2
        while (count >= 4) {
            _mm_storeu_si128((__m128i *)bits, color4);
            bits += 4;
            count -= 4;
        }
#endif

        while (count--) {
            *bits++ = color;
        }
    }

#ifdef UI_DEBUG
    painter->fillCount += UI_RECT_WIDTH(rectangle) * UI_RECT_HEIGHT(rectangle);
#endif
}


bool UIDrawLine(UIPainter *painter, int x0, int y0, int x1, int y1, uint32_t color)
{
    // Apply the clip.

    UIRectangle c = painter->clip;
    if (!UI_RECT_VALID(c))
        return false;
    int       dx = x1 - x0, dy = y1 - y0;
    const int p[4] = {-dx, dx, -dy, dy};
    const int q[4] = {x0 - c.l, c.r - 1 - x0, y0 - c.t, c.b - 1 - y0};
    float     t0 = 0.0f, t1 = 1.0f; // How far along the line the points end up.

    for (int i = 0; i < 4; i++) {
        if (!p[i] && q[i] < 0)
            return false;
        float r = (float)q[i] / p[i];
        if (p[i] < 0 && r > t1)
            return false;
        if (p[i] > 0 && r < t0)
            return false;
        if (p[i] < 0 && r > t0)
            t0 = r;
        if (p[i] > 0 && r < t1)
            t1 = r;
    }

    x1 = x0 + t1 * dx, y1 = y0 + t1 * dy;
    x0 += t0 * dx, y0 += t0 * dy;

    // Calculate the delta X and delta Y.

    if (y1 < y0) {
        int t;
        t = x0, x0 = x1, x1 = t;
        t = y0, y0 = y1, y1 = t;
    }

    dx = x1 - x0, dy = y1 - y0;
    int dxs = dx < 0 ? -1 : 1;
    if (dx < 0)
        dx = -dx;

    // Draw the line using Bresenham's line algorithm.

    uint32_t *bits = painter->bits + y0 * painter->width + x0;

    if (dy * dy < dx * dx) {
        int m = 2 * dy - dx;

        for (int i = 0; i < dx; i++, bits += dxs) {
            *bits = color;
            if (m > 0)
                bits += painter->width, m -= 2 * dx;
            m += 2 * dy;
        }
    } else {
        int m = 2 * dx - dy;

        for (int i = 0; i < dy; i++, bits += painter->width) {
            *bits = color;
            if (m > 0)
                bits += dxs, m -= 2 * dy;
            m += 2 * dx;
        }
    }

    return true;
}


void UIDrawCircle(UIPainter *painter, int cx, int cy, int radius, uint32_t fillColor,
                  uint32_t outlineColor, bool hollow)
{
    // TODO There's a hole missing at the bottom of the circle!
    // TODO This looks bad at small radii (< 20).

    float x = 0, y = -radius;
    float dx = radius, dy = 0;
    float step = 0.2f / radius;
    int   px = 0, py = cy + y;

    while (x >= 0) {
        x += dx * step;
        y += dy * step;
        dx += -x * step;
        dy += -y * step;

        int ix = x, iy = cy + y;

        while (py <= iy) {
            if (py >= painter->clip.t && py < painter->clip.b) {
                for (int s = 0; s <= ix || s <= px; s++) {
                    bool inOutline = ((s <= ix) != (s <= px)) || ((ix == px) && (s == ix));
                    if (hollow && !inOutline)
                        continue;
                    bool clip0 = cx + s >= painter->clip.l && cx + s < painter->clip.r;
                    bool clip1 = cx - s >= painter->clip.l && cx - s < painter->clip.r;
                    if (clip0)
                        painter->bits[painter->width * py + cx + s] =
                            inOutline ? outlineColor : fillColor;
                    if (clip1)
                        painter->bits[painter->width * py + cx - s] =
                            inOutline ? outlineColor : fillColor;
                }
            }

            px = ix, py++;
        }
    }
}


void UIDrawTriangle(UIPainter *painter, int x0, int y0, int x1, int y1, int x2, int y2,
                    uint32_t color)
{
    // Step 1: Sort the points by their y-coordinate.
    if (y1 < y0) {
        int xt = x0;
        x0 = x1, x1 = xt;
        int yt = y0;
        y0 = y1, y1 = yt;
    }
    if (y2 < y1) {
        int xt = x1;
        x1 = x2, x2 = xt;
        int yt = y1;
        y1 = y2, y2 = yt;
    }
    if (y1 < y0) {
        int xt = x0;
        x0 = x1, x1 = xt;
        int yt = y0;
        y0 = y1, y1 = yt;
    }
    if (y2 == y0)
        return;

    // Step 2: Clip the triangle.
    if (x0 < painter->clip.l && x1 < painter->clip.l && x2 < painter->clip.l)
        return;
    if (x0 >= painter->clip.r && x1 >= painter->clip.r && x2 >= painter->clip.r)
        return;
    if (y2 < painter->clip.t || y0 >= painter->clip.b)
        return;
    bool needsXClip = x0 < painter->clip.l + 1 || x0 >= painter->clip.r - 1 ||
                      x1 < painter->clip.l + 1 || x1 >= painter->clip.r - 1 ||
                      x2 < painter->clip.l + 1 || x2 >= painter->clip.r - 1;
    bool needsYClip = y0 < painter->clip.t + 1 || y2 >= painter->clip.b - 1;
#define _UI_DRAW_TRIANGLE_APPLY_CLIP(xo, yo)                                                       \
    if (needsYClip && (yi + yo < painter->clip.t || yi + yo >= painter->clip.b))                   \
        continue;                                                                                  \
    if (needsXClip && xf + xo < painter->clip.l)                                                   \
        xf = painter->clip.l - xo;                                                                 \
    if (needsXClip && xt + xo > painter->clip.r)                                                   \
        xt = painter->clip.r - xo;

    // Step 3: Split into 2 triangles with bases aligned with the x-axis.
    float xm0 = (x2 - x0) * (y1 - y0) / (y2 - y0), xm1 = x1 - x0;
    if (xm1 < xm0) {
        float xmt = xm0;
        xm0 = xm1, xm1 = xmt;
    }
    float xe0 = xm0 + x0 - x2, xe1 = xm1 + x0 - x2;
    int   ym = y1 - y0, ye = y2 - y1;
    float ymr = 1.0f / ym, yer = 1.0f / ye;

    // Step 4: Draw the top part.
    for (float y = 0; y < ym; y++) {
        int xf = xm0 * y * ymr, xt = xm1 * y * ymr, yi = (int)y;
        _UI_DRAW_TRIANGLE_APPLY_CLIP(x0, y0);
        uint32_t *b = &painter->bits[(yi + y0) * painter->width + x0];
        for (int x = xf; x < xt; x++)
            b[x] = color;
    }

    // Step 5: Draw the bottom part.
    for (float y = 0; y < ye; y++) {
        int xf = xe0 * (ye - y) * yer, xt = xe1 * (ye - y) * yer, yi = (int)y;
        _UI_DRAW_TRIANGLE_APPLY_CLIP(x2, y1);
        uint32_t *b = &painter->bits[(yi + y1) * painter->width + x2];
        for (int x = xf; x < xt; x++)
            b[x] = color;
    }
}

void UIDrawTriangleOutline(UIPainter *painter, int x0, int y0, int x1, int y1, int x2, int y2,
                           uint32_t color)
{
    UIDrawLine(painter, x0, y0, x1, y1, color);
    UIDrawLine(painter, x1, y1, x2, y2, color);
    UIDrawLine(painter, x2, y2, x0, y0, color);
}


void UIDrawInvert(UIPainter *painter, UIRectangle rectangle)
{
    rectangle = UIRectangleIntersection(painter->clip, rectangle);

    if (!UI_RECT_VALID(rectangle)) {
        return;
    }

    for (int line = rectangle.t; line < rectangle.b; line++) {
        uint32_t *bits  = painter->bits + line * painter->width + rectangle.l;
        int       count = UI_RECT_WIDTH(rectangle);

        while (count--) {
            uint32_t in = *bits;
            *bits       = in ^ 0xFFFFFF;
            bits++;
        }
    }
}


void UIDrawString(UIPainter *painter, UIRectangle r, const char *string, ptrdiff_t bytes,
                  uint32_t color, int align, UIStringSelection *selection)
{
    UIRectangle oldClip = painter->clip;
    painter->clip       = UIRectangleIntersection(r, oldClip);

    if (!UI_RECT_VALID(painter->clip)) {
        painter->clip = oldClip;
        return;
    }

    if (bytes == -1) {
        bytes = _UIStringLength(string);
    }

    int width  = UIMeasureStringWidth(string, bytes);
    int height = UIMeasureStringHeight();
    int x      = align == UI_ALIGN_CENTER  ? ((r.l + r.r - width) / 2)
                 : align == UI_ALIGN_RIGHT ? (r.r - width)
                                           : r.l;
    int y      = (r.t + r.b - height) / 2;
    int i = 0, j = 0;

    int selectFrom = -1, selectTo = -1;

    if (selection) {
        selectFrom = selection->carets[0];
        selectTo   = selection->carets[1];

        if (selectFrom > selectTo) {
            UI_SWAP(int, selectFrom, selectTo);
        }
    }

    while (j < bytes) {
        ptrdiff_t bytesConsumed = 1;
#ifdef UI_UNICODE
        int c = Utf8GetCodePoint(string, bytes - j, &bytesConsumed);
        UI_ASSERT(bytesConsumed > 0);
        string += bytesConsumed;
#else
        char c = *string++;
#endif
        uint32_t colorText = color;

        if (i >= selectFrom && i < selectTo) {
            int w = ui.activeFont->glyphWidth;
            if (c == '\t') {
                int ii = i;
                while (++ii & 3)
                    w += ui.activeFont->glyphWidth;
            }
            UIDrawBlock(painter, UI_RECT_4(x, x + w, y, y + height), selection->colorBackground);
            colorText = selection->colorText;
        }

        if (c != '\t') {
            UIDrawGlyph(painter, x, y, c, colorText);
        }

        if (selection && selection->carets[0] == i) {
            UIDrawInvert(painter, UI_RECT_4(x, x + 1, y, y + height));
        }

        x += ui.activeFont->glyphWidth, i++;

        if (c == '\t') {
            while (i & 3)
                x += ui.activeFont->glyphWidth, i++;
        }

        j += bytesConsumed;
    }

    if (selection && selection->carets[0] == i) {
        UIDrawInvert(painter, UI_RECT_4(x, x + 1, y, y + height));
    }

    painter->clip = oldClip;
}


void UIDrawBorder(UIPainter *painter, UIRectangle r, uint32_t borderColor, UIRectangle borderSize)
{
    UIDrawBlock(painter, UI_RECT_4(r.l, r.r, r.t, r.t + borderSize.t), borderColor);
    UIDrawBlock(painter, UI_RECT_4(r.l, r.l + borderSize.l, r.t + borderSize.t, r.b - borderSize.b),
                borderColor);
    UIDrawBlock(painter, UI_RECT_4(r.r - borderSize.r, r.r, r.t + borderSize.t, r.b - borderSize.b),
                borderColor);
    UIDrawBlock(painter, UI_RECT_4(r.l, r.r, r.b - borderSize.b, r.b), borderColor);
}


void UIDrawRectangle(UIPainter *painter, UIRectangle r, uint32_t mainColor, uint32_t borderColor,
                     UIRectangle borderSize)
{
    UIDrawBorder(painter, r, borderColor, borderSize);
    UIDrawBlock(
        painter,
        UI_RECT_4(r.l + borderSize.l, r.r - borderSize.r, r.t + borderSize.t, r.b - borderSize.b),
        mainColor);
}


void UIDrawControlDefault(UIPainter *painter, UIRectangle bounds, uint32_t mode, const char *label,
                          ptrdiff_t labelBytes, double position, float scale)
{
    bool     checked       = mode & UI_DRAW_CONTROL_STATE_CHECKED;
    bool     disabled      = mode & UI_DRAW_CONTROL_STATE_DISABLED;
    bool     focused       = mode & UI_DRAW_CONTROL_STATE_FOCUSED;
    bool     hovered       = mode & UI_DRAW_CONTROL_STATE_HOVERED;
    bool     indeterminate = mode & UI_DRAW_CONTROL_STATE_INDETERMINATE;
    bool     pressed       = mode & UI_DRAW_CONTROL_STATE_PRESSED;
    bool     selected      = mode & UI_DRAW_CONTROL_STATE_SELECTED;
    uint32_t which         = mode & UI_DRAW_CONTROL_TYPE_MASK;

    uint32_t buttonColor     = disabled               ? ui.theme.buttonDisabled
                               : (pressed && hovered) ? ui.theme.buttonPressed
                               : (pressed || hovered) ? ui.theme.buttonHovered
                               : focused              ? ui.theme.selected
                                                      : ui.theme.buttonNormal;
    uint32_t buttonTextColor = disabled                           ? ui.theme.textDisabled
                               : buttonColor == ui.theme.selected ? ui.theme.textSelected
                                                                  : ui.theme.text;

    if (which == UI_DRAW_CONTROL_CHECKBOX) {
        uint32_t    color = buttonColor, textColor = buttonTextColor;
        int         midY = (bounds.t + bounds.b) / 2;
        UIRectangle boxBounds =
            UI_RECT_4(bounds.l, bounds.l + UI_SIZE_CHECKBOX_BOX, midY - UI_SIZE_CHECKBOX_BOX / 2,
                      midY + UI_SIZE_CHECKBOX_BOX / 2);
        UIDrawRectangle(painter, boxBounds, color, ui.theme.border, UI_RECT_1(1));
        UIDrawString(painter, UIRectangleAdd(boxBounds, UI_RECT_4(1, 0, 0, 0)),
                     checked         ? "*"
                     : indeterminate ? "-"
                                     : " ",
                     -1, textColor, UI_ALIGN_CENTER, NULL);
        UIDrawString(
            painter,
            UIRectangleAdd(bounds, UI_RECT_4(UI_SIZE_CHECKBOX_BOX + UI_SIZE_CHECKBOX_GAP, 0, 0, 0)),
            label, labelBytes, disabled ? ui.theme.textDisabled : ui.theme.text, UI_ALIGN_LEFT,
            NULL);
    } else if (which == UI_DRAW_CONTROL_MENU_ITEM || which == UI_DRAW_CONTROL_DROP_DOWN ||
               which == UI_DRAW_CONTROL_PUSH_BUTTON) {
        uint32_t color = buttonColor, textColor = buttonTextColor;
        int      borderSize = which == UI_DRAW_CONTROL_MENU_ITEM ? 0 : scale;
        UIDrawRectangle(painter, bounds, color, ui.theme.border, UI_RECT_1(borderSize));

        if (checked && !focused) {
            UIDrawBlock(
                painter,
                UIRectangleAdd(bounds, UI_RECT_1I((int)(UI_SIZE_BUTTON_CHECKED_AREA * scale))),
                ui.theme.buttonPressed);
        }

        UIRectangle innerBounds =
            UIRectangleAdd(bounds, UI_RECT_2I((int)(UI_SIZE_MENU_ITEM_MARGIN * scale), 0));

        if (which == UI_DRAW_CONTROL_MENU_ITEM) {
            if (labelBytes == -1) {
                labelBytes = _UIStringLength(label);
            }

            int tab = 0;
            for (; tab < labelBytes && label[tab] != '\t'; tab++)
                ;

            UIDrawString(painter, innerBounds, label, tab, textColor, UI_ALIGN_LEFT, NULL);

            if (labelBytes > tab) {
                UIDrawString(painter, innerBounds, label + tab + 1, labelBytes - tab - 1, textColor,
                             UI_ALIGN_RIGHT, NULL);
            }
        } else if (which == UI_DRAW_CONTROL_DROP_DOWN) {
            UIDrawString(painter, innerBounds, label, labelBytes, textColor, UI_ALIGN_LEFT, NULL);
            UIDrawString(painter, innerBounds, "\x19", 1, textColor, UI_ALIGN_RIGHT, NULL);
        } else {
            UIDrawString(painter, bounds, label, labelBytes, textColor, UI_ALIGN_CENTER, NULL);
        }
    } else if (which == UI_DRAW_CONTROL_LABEL) {
        UIDrawString(painter, bounds, label, labelBytes, ui.theme.text, UI_ALIGN_LEFT, NULL);
    } else if (which == UI_DRAW_CONTROL_SPLITTER) {
        UIRectangle borders =
            (mode & UI_DRAW_CONTROL_STATE_VERTICAL) ? UI_RECT_2(0, 1) : UI_RECT_2(1, 0);
        UIDrawRectangle(painter, bounds, ui.theme.buttonNormal, ui.theme.border, borders);
    } else if (which == UI_DRAW_CONTROL_SCROLL_TRACK) {
        if (disabled)
            UIDrawBlock(painter, bounds, ui.theme.panel1);
    } else if (which == UI_DRAW_CONTROL_SCROLL_DOWN || which == UI_DRAW_CONTROL_SCROLL_UP) {
        bool     isDown = which == UI_DRAW_CONTROL_SCROLL_DOWN;
        uint32_t color  = pressed   ? ui.theme.buttonPressed
                          : hovered ? ui.theme.buttonHovered
                                    : ui.theme.panel2;
        UIDrawRectangle(painter, bounds, color, ui.theme.border, UI_RECT_1(0));

        if (mode & UI_DRAW_CONTROL_STATE_VERTICAL) {
            UIDrawGlyph(painter, (bounds.l + bounds.r - ui.activeFont->glyphWidth) / 2 + 1,
                        isDown ? (bounds.b - ui.activeFont->glyphHeight - 2 * scale)
                               : (bounds.t + 2 * scale),
                        isDown ? 25 : 24, ui.theme.text);
        } else {
            UIDrawGlyph(painter,
                        isDown ? (bounds.r - ui.activeFont->glyphWidth - 2 * scale)
                               : (bounds.l + 2 * scale),
                        (bounds.t + bounds.b - ui.activeFont->glyphHeight) / 2, isDown ? 26 : 27,
                        ui.theme.text);
        }
    } else if (which == UI_DRAW_CONTROL_SCROLL_THUMB) {
        uint32_t color = pressed   ? ui.theme.buttonPressed
                         : hovered ? ui.theme.buttonHovered
                                   : ui.theme.buttonNormal;
        UIDrawRectangle(painter, bounds, color, ui.theme.border, UI_RECT_1(2));
    } else if (which == UI_DRAW_CONTROL_GAUGE) {
        UIDrawRectangle(painter, bounds, ui.theme.buttonNormal, ui.theme.border, UI_RECT_1(1));
        UIRectangle filled = UIRectangleAdd(bounds, UI_RECT_1I(1));
        if (mode & UI_DRAW_CONTROL_STATE_VERTICAL) {
            filled.t = filled.b - UI_RECT_HEIGHT(filled) * position;
        } else {
            filled.r = filled.l + UI_RECT_WIDTH(filled) * position;
        }
        UIDrawBlock(painter, filled, ui.theme.selected);
    } else if (which == UI_DRAW_CONTROL_SLIDER) {
        bool        vertical      = mode & UI_DRAW_CONTROL_STATE_VERTICAL;
        int         centerX       = (bounds.r + bounds.l) / 2;
        int         centerY       = (bounds.t + bounds.b) / 2;
        int         center        = vertical ? centerX : centerY;
        int         trackSize     = UI_SIZE_SLIDER_TRACK * scale;
        int         thumbSize     = UI_SIZE_SLIDER_THUMB * scale;
        int         thumbPosition = vertical ? (UI_RECT_HEIGHT(bounds) - thumbSize) * position
                                             : (UI_RECT_WIDTH(bounds) - thumbSize) * position;
        UIRectangle track         = vertical ? UI_RECT_4(center - (trackSize + 1) / 2,
                                                         center + trackSize / 2, bounds.t, bounds.b)
                                             : UI_RECT_4(bounds.l, bounds.r, center - (trackSize + 1) / 2,
                                                         center + trackSize / 2);
        UIDrawRectangle(painter, track, disabled ? ui.theme.buttonDisabled : ui.theme.buttonNormal,
                        ui.theme.border, UI_RECT_1(1));
        uint32_t    color = disabled  ? ui.theme.buttonDisabled
                            : pressed ? ui.theme.buttonPressed
                            : hovered ? ui.theme.buttonHovered
                                      : ui.theme.buttonNormal;
        UIRectangle thumb =
            vertical ? UI_RECT_4(center - (thumbSize + 1) / 2, center + thumbSize / 2,
                                 bounds.b - thumbPosition - thumbSize, bounds.b - thumbPosition)
                     : UI_RECT_4(bounds.l + thumbPosition, bounds.l + thumbPosition + thumbSize,
                                 center - (thumbSize + 1) / 2, center + thumbSize / 2);
        UIDrawRectangle(painter, thumb, color, ui.theme.border, UI_RECT_1(1));
    } else if (which == UI_DRAW_CONTROL_TEXTBOX) {
        UIDrawRectangle(painter, bounds,
                        disabled  ? ui.theme.buttonDisabled
                        : focused ? ui.theme.textboxFocused
                                  : ui.theme.textboxNormal,
                        ui.theme.border, UI_RECT_1(1));
    } else if (which == UI_DRAW_CONTROL_MODAL_POPUP) {
        UIRectangle bounds2 = UIRectangleAdd(bounds, UI_RECT_1I(-1));
        UIDrawBorder(painter, bounds2, ui.theme.border, UI_RECT_1(1));
        UIDrawBorder(painter, UIRectangleAdd(bounds2, UI_RECT_1(1)), ui.theme.border, UI_RECT_1(1));
    } else if (which == UI_DRAW_CONTROL_MENU) {
        UIDrawBlock(painter, bounds, ui.theme.border);
    } else if (which == UI_DRAW_CONTROL_TABLE_ROW) {
        if (selected)
            UIDrawBlock(painter, bounds, ui.theme.selected);
        else if (hovered)
            UIDrawBlock(painter, bounds, ui.theme.buttonHovered);
    } else if (which == UI_DRAW_CONTROL_TABLE_CELL) {
        uint32_t textColor = selected ? ui.theme.textSelected : ui.theme.text;
        UIDrawString(painter, bounds, label, labelBytes, textColor, UI_ALIGN_LEFT, NULL);
    } else if (which == UI_DRAW_CONTROL_TABLE_BACKGROUND) {
        UIDrawBlock(painter, bounds, ui.theme.panel2);
        UIDrawRectangle(
            painter,
            UI_RECT_4(bounds.l, bounds.r, bounds.t, bounds.t + (int)(UI_SIZE_TABLE_HEADER * scale)),
            ui.theme.panel1, ui.theme.border, UI_RECT_4(0, 0, 0, 1));
    } else if (which == UI_DRAW_CONTROL_TABLE_HEADER) {
        UIDrawString(painter, bounds, label, labelBytes, ui.theme.text, UI_ALIGN_LEFT, NULL);
        if (selected)
            UIDrawInvert(painter, bounds);
    } else if (which == UI_DRAW_CONTROL_MDI_CHILD) {
        UI_MDI_CHILD_CALCULATE_LAYOUT(bounds, scale);
        UIRectangle borders = UI_RECT_4(borderSize, borderSize, titleSize, borderSize);
        UIDrawBorder(painter, bounds, ui.theme.buttonNormal, borders);
        UIDrawBorder(painter, bounds, ui.theme.border, UI_RECT_1((int)scale));
        UIDrawBorder(painter, UIRectangleAdd(content, UI_RECT_1I(-1)), ui.theme.border,
                     UI_RECT_1((int)scale));
        UIDrawString(painter, title, label, labelBytes, ui.theme.text, UI_ALIGN_LEFT, NULL);
    } else if (which == UI_DRAW_CONTROL_TAB) {
        uint32_t    color = selected ? ui.theme.buttonPressed : ui.theme.buttonNormal;
        UIRectangle t     = bounds;
        if (selected)
            t.b++, t.t--;
        else
            t.t++;
        UIDrawRectangle(painter, t, color, ui.theme.border, UI_RECT_1(1));
        UIDrawString(painter, bounds, label, labelBytes, ui.theme.text, UI_ALIGN_CENTER, NULL);
    } else if (which == UI_DRAW_CONTROL_TAB_BAND) {
        UIDrawRectangle(painter, bounds, ui.theme.panel1, ui.theme.border, UI_RECT_4(0, 0, 0, 1));
    }
}
