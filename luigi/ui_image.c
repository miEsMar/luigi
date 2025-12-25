#include "ui_image.h"
#include "ui_window.h"
#include "utils.h"


void _UIImageDisplayUpdateViewport(UIImageDisplay *display)
{
    UIRectangle bounds = display->e.bounds;
    bounds.r -= bounds.l, bounds.b -= bounds.t;

    float minimumZoomX = 1, minimumZoomY = 1;
    if (display->width > bounds.r)
        minimumZoomX = (float)bounds.r / display->width;
    if (display->height > bounds.b)
        minimumZoomY = (float)bounds.b / display->height;
    float minimumZoom = minimumZoomX < minimumZoomY ? minimumZoomX : minimumZoomY;

    if (display->zoom < minimumZoom || (display->e.flags & _UI_IMAGE_DISPLAY_ZOOM_FIT)) {
        display->zoom = minimumZoom;
        display->e.flags |= _UI_IMAGE_DISPLAY_ZOOM_FIT;
    }

    if (display->panX < 0)
        display->panX = 0;
    if (display->panY < 0)
        display->panY = 0;
    if (display->panX > display->width - bounds.r / display->zoom)
        display->panX = display->width - bounds.r / display->zoom;
    if (display->panY > display->height - bounds.b / display->zoom)
        display->panY = display->height - bounds.b / display->zoom;

    if (bounds.r && display->width * display->zoom <= bounds.r)
        display->panX = display->width / 2 - bounds.r / display->zoom / 2;
    if (bounds.b && display->height * display->zoom <= bounds.b)
        display->panY = display->height / 2 - bounds.b / display->zoom / 2;
}


int _UIImageDisplayMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIImageDisplay *display = (UIImageDisplay *)element;

    if (message == UI_MSG_GET_HEIGHT) {
        return display->height;
    } else if (message == UI_MSG_GET_WIDTH) {
        return display->width;
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(display->bits);
    } else if (message == UI_MSG_PAINT) {
        UIPainter *painter = (UIPainter *)dp;

        int w = UI_RECT_WIDTH(element->bounds), h = UI_RECT_HEIGHT(element->bounds);
        int x = _UILinearMap(0, display->panX, display->panX + w / display->zoom, 0, w) +
                element->bounds.l;
        int y = _UILinearMap(0, display->panY, display->panY + h / display->zoom, 0, h) +
                element->bounds.t;

        UIRectangle image  = UI_RECT_4(x, x + (int)(display->width * display->zoom), y,
                                       (int)(y + display->height * display->zoom));
        UIRectangle bounds = UIRectangleIntersection(
            painter->clip, UIRectangleIntersection(display->e.bounds, image));
        if (!UI_RECT_VALID(bounds))
            return 0;

#ifdef UI_AVX512
# define VCOUNT (16)
# define VSIZE  (4)

# define vfloat __m512
# define VFSet1 _mm512_set1_ps
# define VFSetN(x)                                                                                 \
     _mm512_set_ps((x) + 15, (x) + 14, (x) + 13, (x) + 12, (x) + 11, (x) + 10, (x) + 9, (x) + 8,   \
                   (x) + 7, (x) + 6, (x) + 5, (x) + 4, (x) + 3, (x) + 2, (x) + 1, (x) + 0)
# define VFAdd            _mm512_add_ps
# define VFSub            _mm512_sub_ps
# define VFMul            _mm512_mul_ps
# define VFMulAdd         _mm512_fmadd_ps
# define VFRoundPosInf(x) _mm512_cvt_roundps_epi32((x), _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)
# define VFRoundNegInf(x) _mm512_cvt_roundps_epi32((x), _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC)
# define VFRoundZero(x)   _mm512_cvt_roundps_epi32((x), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)

# define vint   __m512i
# define VISet1 _mm512_set1_epi32
# define VISetN(x)                                                                                 \
     _mm512_set_epi32((x) + 15, (x) + 14, (x) + 13, (x) + 12, (x) + 11, (x) + 10, (x) + 9,         \
                      (x) + 8, (x) + 7, (x) + 6, (x) + 5, (x) + 4, (x) + 3, (x) + 2, (x) + 1,      \
                      (x) + 0)
# define VIZero                      _mm512_setzero_epi32
# define VIAdd                       _mm512_add_epi32
# define VIAnd                       _mm512_and_epi32
# define VIOr                        _mm512_or_epi32
# define VIMax                       _mm512_max_epi32
# define VIMin                       _mm512_min_epi32
# define VIShr(x, y)                 _mm512_shrdi_epi32((x), VIZero(), (y))
# define VIShl(x, y)                 _mm512_shldi_epi32((x), VIZero(), (y))
# define VICastFloat(x)              _mm512_cvt_roundepi32_ps((x), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)
# define VILoad(base, index)         _mm512_i32gather_epi32((index), (base), VSIZE)
# define VIStore(base, index, value) _mm512_i32scatter_epi32((base), (index), (value), VSIZE)
#endif

        if (display->zoom == 1) {
            uint32_t *lineStart = (uint32_t *)painter->bits + bounds.t * painter->width + bounds.l;
            uint32_t *sourceLineStart =
                display->bits + (bounds.l - image.l) + display->width * (bounds.t - image.t);

            for (int i = 0; i < bounds.b - bounds.t;
                 i++, lineStart += painter->width, sourceLineStart += display->width) {
                uint32_t *destination = lineStart;
                uint32_t *source      = sourceLineStart;
                int       j           = bounds.r - bounds.l;

#ifdef UI_AVX512
                vint index = VISetN(0);

                while (j >= VCOUNT) {
                    VIStore(destination, index, VILoad(source, index));
                    j -= VCOUNT;
                    destination += VCOUNT;
                    source += VCOUNT;
                }
#endif

                while (j) {
                    *destination = *source;
                    j--;
                    destination++;
                    source++;
                }
            }
        } else if (element->flags & UI_IMAGE_DISPLAY_HQ_ZOOM_IN) {
            float     zr          = 1.0f / display->zoom;
            uint32_t *destination = (uint32_t *)painter->bits;

            for (int i = bounds.t; i < bounds.b; i++) {
                float ty  = (i - image.t) * zr - 0.5f;
                int   ty0 = floorf(ty);
                int   ty1 = ceilf(ty);
                float tyf = ty - floorf(ty);
                if (ty0 < 0) {
                    ty0 = ty1;
                }
                if (ty1 >= display->height) {
                    ty1 = ty0;
                }

                int j = bounds.l;

#ifdef UI_AVX512
                int    j0 = j - image.l;
                vfloat J  = VFSetN(j0);
                vint   Ji = VISetN(j);

                while (j <= bounds.r - VCOUNT) {
                    vfloat tx  = VFMulAdd(J, VFSet1(zr), VFSet1(-0.5f));
                    vint   tx0 = VFRoundNegInf(tx);
                    vint   tx1 = VFRoundPosInf(tx);
                    vfloat txf = VFSub(tx, VICastFloat(tx0));
                    tx0        = VIMax(tx0, VIZero());
                    tx1        = VIMin(tx1, VISet1(display->width - 1));

# define GATHER_PIXEL(txv, tyv)                                                                    \
     (VILoad(display->bits, VIAdd(VISet1((tyv) * display->width), (txv))))
                    vint s00 = GATHER_PIXEL(tx0, ty0);
                    vint s10 = GATHER_PIXEL(tx1, ty0);
                    vint s01 = GATHER_PIXEL(tx0, ty1);
                    vint s11 = GATHER_PIXEL(tx1, ty1);

# define SPLIT_PIXEL(source, shift) (VICastFloat(VIAnd(VIShr((source), (shift)), VISet1(0xFF))))
                    vfloat s000 = SPLIT_PIXEL(s00, 0);
                    vfloat s001 = SPLIT_PIXEL(s00, 8);
                    vfloat s002 = SPLIT_PIXEL(s00, 16);
                    vfloat s100 = SPLIT_PIXEL(s10, 0);
                    vfloat s101 = SPLIT_PIXEL(s10, 8);
                    vfloat s102 = SPLIT_PIXEL(s10, 16);
                    vfloat s010 = SPLIT_PIXEL(s01, 0);
                    vfloat s011 = SPLIT_PIXEL(s01, 8);
                    vfloat s012 = SPLIT_PIXEL(s01, 16);
                    vfloat s110 = SPLIT_PIXEL(s11, 0);
                    vfloat s111 = SPLIT_PIXEL(s11, 8);
                    vfloat s112 = SPLIT_PIXEL(s11, 16);

# define LERP_HORZ(from, to) (VFMulAdd(VFSub((to), (from)), txf, from))
                    vfloat m00 = LERP_HORZ(s000, s100);
                    vfloat m01 = LERP_HORZ(s001, s101);
                    vfloat m02 = LERP_HORZ(s002, s102);
                    vfloat m10 = LERP_HORZ(s010, s110);
                    vfloat m11 = LERP_HORZ(s011, s111);
                    vfloat m12 = LERP_HORZ(s012, s112);

# define LERP_VERT(from, to) (VFMulAdd(VFSub((to), (from)), VFSet1(tyf), from))
                    vfloat m0 = LERP_VERT(m00, m10);
                    vfloat m1 = LERP_VERT(m01, m11);
                    vfloat m2 = LERP_VERT(m02, m12);
                    vint   m  = VIOr(VIOr(VFRoundZero(m0), VIShl(VFRoundZero(m1), 8)),
                                     VIShl(VFRoundZero(m2), 16));

                    VIStore(destination + i * painter->width, Ji, m);

                    j += VCOUNT;
                    J += VFSet1(VCOUNT);
                    Ji += VISet1(VCOUNT);
                }
#endif

                while (j <= bounds.r - 1) {
                    float tx  = (j - image.l) * zr - 0.5f;
                    int   tx0 = floorf(tx);
                    int   tx1 = ceilf(tx);
                    float txf = tx - floorf(tx);
                    if (tx0 < 0) {
                        tx0 = tx1;
                    }
                    if (tx1 >= display->width) {
                        tx1 = tx0;
                    }

                    uint32_t s00 = display->bits[ty0 * display->width + tx0];
                    uint32_t s10 = display->bits[ty0 * display->width + tx1];
                    uint32_t s01 = display->bits[ty1 * display->width + tx0];
                    uint32_t s11 = display->bits[ty1 * display->width + tx1];

                    int32_t s000 = (s00 >> 0) & 0xFF, s001 = (s00 >> 8) & 0xFF,
                            s002 = (s00 >> 16) & 0xFF;
                    int32_t s100 = (s10 >> 0) & 0xFF, s101 = (s10 >> 8) & 0xFF,
                            s102 = (s10 >> 16) & 0xFF;
                    int32_t s010 = (s01 >> 0) & 0xFF, s011 = (s01 >> 8) & 0xFF,
                            s012 = (s01 >> 16) & 0xFF;
                    int32_t s110 = (s11 >> 0) & 0xFF, s111 = (s11 >> 8) & 0xFF,
                            s112 = (s11 >> 16) & 0xFF;

                    int32_t m00 = (s100 - s000) * txf + s000, m01 = (s101 - s001) * txf + s001,
                            m02 = (s102 - s002) * txf + s002;
                    int32_t m10 = (s110 - s010) * txf + s010, m11 = (s111 - s011) * txf + s011,
                            m12 = (s112 - s012) * txf + s012;
                    int32_t m0 = (m10 - m00) * tyf + m00, m1 = (m11 - m01) * tyf + m01,
                            m2 = (m12 - m02) * tyf + m02;
                    int32_t m  = m0 | (m1 << 8) | (m2 << 16);

                    destination[i * painter->width + j] = m;
                    j++;
                }
            }
        } else {
            float     zr          = 1.0f / display->zoom;
            uint32_t *destination = (uint32_t *)painter->bits;

            for (int i = bounds.t; i < bounds.b; i++) {
                int ty = (i - image.t) * zr;
                int j  = bounds.l;

#ifdef UI_AVX512
                vint   J  = VISetN(j);
                vfloat J0 = VFSetN(j - image.l);

                while (j <= bounds.r - VCOUNT) {
                    vint tx = VFRoundZero(VFMul(J0, VFSet1(zr)));
                    VIStore(destination + i * painter->width, J,
                            VILoad(display->bits + ty * display->width, tx));
                    j += VCOUNT;
                    J += VISet1(VCOUNT);
                    J0 += VFSet1(VCOUNT);
                }
#endif

                while (j <= bounds.r - 1) {
                    int tx                              = (j - image.l) * zr;
                    destination[i * painter->width + j] = display->bits[ty * display->width + tx];
                    j++;
                }
            }
        }
    } else if (message == UI_MSG_MOUSE_WHEEL && (element->flags & UI_IMAGE_DISPLAY_INTERACTIVE)) {
        display->e.flags &= ~_UI_IMAGE_DISPLAY_ZOOM_FIT;
        int   divisions   = -di / 72;
        float factor      = 1;
        float perDivision = element->window->ctrl ? 2.0f : element->window->alt ? 1.01f : 1.2f;
        while (divisions > 0)
            factor *= perDivision, divisions--;
        while (divisions < 0)
            factor /= perDivision, divisions++;
        if (display->zoom * factor > 64)
            factor = 64 / display->zoom;
        int mx = element->window->cursorX - element->bounds.l;
        int my = element->window->cursorY - element->bounds.t;
        display->zoom *= factor;
        display->panX -= mx / display->zoom * (1 - factor);
        display->panY -= my / display->zoom * (1 - factor);
        _UIImageDisplayUpdateViewport(display);
        UIElementRepaint(&display->e, NULL);
    } else if (message == UI_MSG_LAYOUT && (element->flags & UI_IMAGE_DISPLAY_INTERACTIVE)) {
        UIRectangle bounds = display->e.bounds;
        bounds.r -= bounds.l, bounds.b -= bounds.t;
        display->panX -= (bounds.r - display->previousWidth) / 2 / display->zoom;
        display->panY -= (bounds.b - display->previousHeight) / 2 / display->zoom;
        display->previousWidth = bounds.r, display->previousHeight = bounds.b;
        _UIImageDisplayUpdateViewport(display);
    } else if (message == UI_MSG_GET_CURSOR && (element->flags & UI_IMAGE_DISPLAY_INTERACTIVE) &&
               (UI_RECT_WIDTH(element->bounds) < display->width * display->zoom ||
                UI_RECT_HEIGHT(element->bounds) < display->height * display->zoom)) {
        return UI_CURSOR_HAND;
    } else if (message == UI_MSG_MOUSE_DRAG &&
               element->window->pressedButton ==
                   ((element->flags & UI_IMAGE_DISPLAY_MIDDLE_DRAG_TO_PAN) ? 2 : 1)) {
        display->panX -= (element->window->cursorX - display->previousPanPointX) / display->zoom;
        display->panY -= (element->window->cursorY - display->previousPanPointY) / display->zoom;
        _UIImageDisplayUpdateViewport(display);
        display->previousPanPointX = element->window->cursorX;
        display->previousPanPointY = element->window->cursorY;
        UIElementRepaint(element, NULL);
    } else if (message == ((element->flags & UI_IMAGE_DISPLAY_MIDDLE_DRAG_TO_PAN)
                               ? UI_MSG_MIDDLE_DOWN
                               : UI_MSG_LEFT_DOWN)) {
        display->e.flags &= ~_UI_IMAGE_DISPLAY_ZOOM_FIT;
        display->previousPanPointX = element->window->cursorX;
        display->previousPanPointY = element->window->cursorY;
    }

    return 0;
}


//


void UIImageDisplaySetContent(UIImageDisplay *display, uint32_t *bits, size_t width, size_t height,
                              size_t stride)
{
    UI_FREE(display->bits);

    display->bits   = (uint32_t *)UI_MALLOC(width * height * 4);
    display->width  = width;
    display->height = height;

    uint32_t *destination = display->bits;
    uint32_t *source      = bits;

    for (uintptr_t row = 0; row < height; row++, source += stride / 4) {
        for (uintptr_t i = 0; i < width; i++) {
            *destination++ = source[i];
        }
    }

    UIElementMeasurementsChanged(&display->e, 3);
    UIElementRepaint(&display->e, NULL);
}


UIImageDisplay *UIImageDisplayCreate(UIElement *parent, uint32_t flags, uint32_t *bits,
                                     size_t width, size_t height, size_t stride)
{
    UIImageDisplay *display = (UIImageDisplay *)UIElementCreate(
        sizeof(UIImageDisplay), parent, flags, _UIImageDisplayMessage, "ImageDisplay");
    display->zoom = 1.0f;
    UIImageDisplaySetContent(display, bits, width, height, stride);
    return display;
}
