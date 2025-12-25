#include "ui_rect.h"


UIRectangle UIRectangleIntersection(UIRectangle a, UIRectangle b)
{
    if (a.l < b.l)
        a.l = b.l;
    if (a.t < b.t)
        a.t = b.t;
    if (a.r > b.r)
        a.r = b.r;
    if (a.b > b.b)
        a.b = b.b;
    return a;
}

UIRectangle UIRectangleBounding(UIRectangle a, UIRectangle b)
{
    if (a.l > b.l)
        a.l = b.l;
    if (a.t > b.t)
        a.t = b.t;
    if (a.r < b.r)
        a.r = b.r;
    if (a.b < b.b)
        a.b = b.b;
    return a;
}

UIRectangle UIRectangleAdd(UIRectangle a, UIRectangle b)
{
    a.l += b.l;
    a.t += b.t;
    a.r += b.r;
    a.b += b.b;
    return a;
}

UIRectangle UIRectangleTranslate(UIRectangle a, UIRectangle b)
{
    a.l += b.l;
    a.t += b.t;
    a.r += b.l;
    a.b += b.t;
    return a;
}

UIRectangle UIRectangleCenter(UIRectangle parent, UIRectangle child)
{
    int childWidth = UI_RECT_WIDTH(child), childHeight = UI_RECT_HEIGHT(child);
    int parentWidth = UI_RECT_WIDTH(parent), parentHeight = UI_RECT_HEIGHT(parent);
    child.l = parentWidth / 2 - childWidth / 2 + parent.l, child.r = child.l + childWidth;
    child.t = parentHeight / 2 - childHeight / 2 + parent.t, child.b = child.t + childHeight;
    return child;
}

UIRectangle UIRectangleFit(UIRectangle parent, UIRectangle child, bool allowScalingUp)
{
    int childWidth = UI_RECT_WIDTH(child), childHeight = UI_RECT_HEIGHT(child);
    int parentWidth = UI_RECT_WIDTH(parent), parentHeight = UI_RECT_HEIGHT(parent);

    if (childWidth < parentWidth && childHeight < parentHeight && !allowScalingUp) {
        return UIRectangleCenter(parent, child);
    }

    float childAspectRatio   = (float)childWidth / childHeight;
    int   childMaximumWidth  = parentHeight * childAspectRatio;
    int   childMaximumHeight = parentWidth / childAspectRatio;

    if (childMaximumWidth > parentWidth) {
        return UIRectangleCenter(parent, UI_RECT_2S(parentWidth, childMaximumHeight));
    } else {
        return UIRectangleCenter(parent, UI_RECT_2S(childMaximumWidth, parentHeight));
    }
}
