#ifndef LUIGI_RECT_H_
#define LUIGI_RECT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>


#ifdef UI_ESSENCE
# define UIRectangle EsRectangle
#else
typedef struct UIRectangle {
    int l, r, t, b;
} UIRectangle;
#endif


#define UI_RECT_4(x, y, z, w)                                                                      \
    (UIRectangle) { (x), (y), (z), (w) }
#define UI_RECT_1(x)             UI_RECT_4(x, x, x, x)
#define UI_RECT_1I(x)            UI_RECT_4(x, -x, x, -x)
#define UI_RECT_2(x, y)          UI_RECT_4(x, x, y, y)
#define UI_RECT_2I(x, y)         UI_RECT_4(x, -x, y, -y)
#define UI_RECT_2S(x, y)         UI_RECT_4(0, x, 0, y)
#define UI_RECT_4PD(x, y, w, h)  UI_RECT_4(x, x + w, y, y + h)
#define UI_RECT_WIDTH(_r)        ((_r).r - (_r).l)
#define UI_RECT_HEIGHT(_r)       ((_r).b - (_r).t)
#define UI_RECT_TOTAL_H(_r)      ((_r).r + (_r).l)
#define UI_RECT_TOTAL_V(_r)      ((_r).b + (_r).t)
#define UI_RECT_SIZE(_r)         UI_RECT_WIDTH(_r), UI_RECT_HEIGHT(_r)
#define UI_RECT_TOP_LEFT(_r)     (_r).l, (_r).t
#define UI_RECT_BOTTOM_LEFT(_r)  (_r).l, (_r).b
#define UI_RECT_BOTTOM_RIGHT(_r) (_r).r, (_r).b
#define UI_RECT_ALL(_r)          (_r).l, (_r).r, (_r).t, (_r).b
#define UI_RECT_VALID(_r)        ((_r).l < (_r).r && (_r).t < (_r).b)


#define UIRectangleEquals(r1, r2)                                                                  \
    ((r1).l == (r2).l && (r1).r == (r2).r && (r1).t == (r2).t && (r1).b == (r2).b)


#define UIRectangleContains(a, x, y) ((a).l <= x && (a).r > x && (a).t <= y && (a).b > y)


//


UIRectangle UIRectangleIntersection(UIRectangle a, UIRectangle b);
UIRectangle UIRectangleBounding(UIRectangle a, UIRectangle b);
UIRectangle UIRectangleAdd(UIRectangle a, UIRectangle b);
UIRectangle UIRectangleTranslate(UIRectangle a, UIRectangle b);
UIRectangle UIRectangleCenter(UIRectangle parent, UIRectangle child);
UIRectangle UIRectangleFit(UIRectangle parent, UIRectangle child, bool allowScalingUp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_RECT_H_
