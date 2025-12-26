#ifndef LUIGI_SCROLL_H_
#define LUIGI_SCROLL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "timing.h"
#include "ui_element.h"


#define UI_SIZE_SCROLL_BAR           (16)
#define UI_SIZE_SCROLL_MINIMUM_THUMB (20)

#define UI_SCROLL_BAR_HORIZONTAL (1 << 0)


typedef struct UIScrollBar {
    UIElement  e;
    int64_t    maximum, page;
    int64_t    dragOffset;
    double     position;
    UI_CLOCK_T lastAnimateTime;
    bool       inDrag, horizontal;
} UIScrollBar;

#define _UI_LAYOUT_SCROLL_BAR_PAIR(element)                                                        \
    element->vScroll->page =                                                                       \
        vSpace - (element->hScroll->page < element->hScroll->maximum ? scrollBarSize : 0);         \
    element->hScroll->page =                                                                       \
        hSpace - (element->vScroll->page < element->vScroll->maximum ? scrollBarSize : 0);         \
    element->vScroll->page =                                                                       \
        vSpace - (element->hScroll->page < element->hScroll->maximum ? scrollBarSize : 0);         \
    UIRectangle vScrollBarBounds = element->e.bounds, hScrollBarBounds = element->e.bounds;        \
    hScrollBarBounds.r = vScrollBarBounds.l =                                                      \
        vScrollBarBounds.r -                                                                       \
        (element->vScroll->page < element->vScroll->maximum ? scrollBarSize : 0);                  \
    vScrollBarBounds.b = hScrollBarBounds.t =                                                      \
        hScrollBarBounds.b -                                                                       \
        (element->hScroll->page < element->hScroll->maximum ? scrollBarSize : 0);                  \
    UIElementMove(&element->vScroll->e, vScrollBarBounds, true);                                   \
    UIElementMove(&element->hScroll->e, hScrollBarBounds, true);

#define _UI_KEY_INPUT_VSCROLL(element, rowHeight, pageHeight)                                      \
    if (m->code == UI_KEYCODE_UP)                                                                  \
        element->vScroll->position -= (rowHeight);                                                 \
    else if (m->code == UI_KEYCODE_DOWN)                                                           \
        element->vScroll->position += (rowHeight);                                                 \
    else if (m->code == UI_KEYCODE_PAGE_UP)                                                        \
        element->vScroll->position += (pageHeight);                                                \
    else if (m->code == UI_KEYCODE_PAGE_DOWN)                                                      \
        element->vScroll->position -= (pageHeight);                                                \
    else if (m->code == UI_KEYCODE_HOME)                                                           \
        element->vScroll->position = 0;                                                            \
    else if (m->code == UI_KEYCODE_END)                                                            \
        element->vScroll->position = element->vScroll->maximum;                                    \
    UIElementRefresh(&element->e);


//


UIScrollBar *UIScrollBarCreate(UIElement *parent, uint32_t flags);


#ifdef __cplusplus
}
#endif

#endif // LUIGI_SCROLL_H_
