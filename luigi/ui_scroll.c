#include "ui_scroll.h"
#include "ui_animation.h"
#include "ui_draw.h"
#include "ui_element.h"
#include "ui_event.h"
#include "ui_window.h"


static int _UIScrollBarMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIScrollBar *scrollBar = (UIScrollBar *)element;

    if (message == UI_MSG_GET_WIDTH || message == UI_MSG_GET_HEIGHT) {
        return UI_SIZE_SCROLL_BAR * element->window->scale;
    } else if (message == UI_MSG_LAYOUT) {
        UIElement *up    = element->children[0];
        UIElement *thumb = element->children[1];
        UIElement *down  = element->children[2];

        if (scrollBar->page >= scrollBar->maximum || scrollBar->maximum <= 0 ||
            scrollBar->page <= 0) {
            up->flags |= UI_ELEMENT_HIDE;
            thumb->flags |= UI_ELEMENT_HIDE;
            down->flags |= UI_ELEMENT_HIDE;

            scrollBar->position = 0;
        } else {
            up->flags &= ~UI_ELEMENT_HIDE;
            thumb->flags &= ~UI_ELEMENT_HIDE;
            down->flags &= ~UI_ELEMENT_HIDE;

            int size      = scrollBar->horizontal ? UI_RECT_WIDTH(element->bounds)
                                                  : UI_RECT_HEIGHT(element->bounds);
            int thumbSize = size * scrollBar->page / scrollBar->maximum;

            if (thumbSize < UI_SIZE_SCROLL_MINIMUM_THUMB * element->window->scale) {
                thumbSize = UI_SIZE_SCROLL_MINIMUM_THUMB * element->window->scale;
            }

            if (scrollBar->position < 0) {
                scrollBar->position = 0;
            } else if (scrollBar->position > scrollBar->maximum - scrollBar->page) {
                scrollBar->position = scrollBar->maximum - scrollBar->page;
            }

            int thumbPosition =
                scrollBar->position / (scrollBar->maximum - scrollBar->page) * (size - thumbSize);

            if (scrollBar->position == scrollBar->maximum - scrollBar->page) {
                thumbPosition = size - thumbSize;
            }

            if (scrollBar->horizontal) {
                UIRectangle r = element->bounds;
                r.r           = r.l + thumbPosition;
                UIElementMove(up, r, false);
                r.l = r.r, r.r = r.l + thumbSize;
                UIElementMove(thumb, r, false);
                r.l = r.r, r.r = element->bounds.r;
                UIElementMove(down, r, false);
            } else {
                UIRectangle r = element->bounds;
                r.b           = r.t + thumbPosition;
                UIElementMove(up, r, false);
                r.t = r.b, r.b = r.t + thumbSize;
                UIElementMove(thumb, r, false);
                r.t = r.b, r.b = element->bounds.b;
                UIElementMove(down, r, false);
            }
        }
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_SCROLL_TRACK |
                          ((scrollBar->page >= scrollBar->maximum || scrollBar->maximum <= 0 ||
                            scrollBar->page <= 0)
                               ? UI_DRAW_CONTROL_STATE_DISABLED
                               : 0),
                      NULL, 0, 0, element->window->scale);
    } else if (message == UI_MSG_MOUSE_WHEEL) {
        scrollBar->position += di;
        UIElementRefresh(element);
        UIElementMessage(element->parent, UI_MSG_SCROLLED, 0, 0);
        return 1;
    }

    return 0;
}


static int _UIScrollUpDownMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIScrollBar *scrollBar = (UIScrollBar *)element->parent;
    bool         isDown    = element->cp;

    if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      (isDown ? UI_DRAW_CONTROL_SCROLL_DOWN : UI_DRAW_CONTROL_SCROLL_UP) |
                          (scrollBar->horizontal ? 0 : UI_DRAW_CONTROL_STATE_VERTICAL) |
                          UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      NULL, 0, 0, element->window->scale);
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_LEFT_DOWN) {
        UIElementAnimate(element, false);
        scrollBar->lastAnimateTime = UI_CLOCK();
    } else if (message == UI_MSG_LEFT_UP) {
        UIElementAnimate(element, true);
    } else if (message == UI_MSG_ANIMATE) {
        UI_CLOCK_T previous     = scrollBar->lastAnimateTime;
        UI_CLOCK_T current      = UI_CLOCK();
        UI_CLOCK_T delta        = current - previous;
        double     deltaSeconds = (double)delta / UI_CLOCKS_PER_SECOND;
        if (deltaSeconds > 0.1)
            deltaSeconds = 0.1;
        double deltaPixels         = deltaSeconds * scrollBar->page * 3;
        scrollBar->lastAnimateTime = current;
        if (isDown)
            scrollBar->position += deltaPixels;
        else
            scrollBar->position -= deltaPixels;
        UIElementRefresh(&scrollBar->e);
        UIElementMessage(scrollBar->e.parent, UI_MSG_SCROLLED, 0, 0);
    }

    return 0;
}


static int _UIScrollThumbMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIScrollBar *scrollBar = (UIScrollBar *)element->parent;

    if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_SCROLL_THUMB |
                          (scrollBar->horizontal ? 0 : UI_DRAW_CONTROL_STATE_VERTICAL) |
                          UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      NULL, 0, 0, element->window->scale);
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_MOUSE_DRAG && element->window->pressedButton == 1) {
        if (!scrollBar->inDrag) {
            scrollBar->inDrag = true;

            if (scrollBar->horizontal) {
                scrollBar->dragOffset =
                    element->bounds.l - scrollBar->e.bounds.l - element->window->cursorX;
            } else {
                scrollBar->dragOffset =
                    element->bounds.t - scrollBar->e.bounds.t - element->window->cursorY;
            }
        }

        int thumbPosition =
            (scrollBar->horizontal ? element->window->cursorX : element->window->cursorY) +
            scrollBar->dragOffset;
        int size            = scrollBar->horizontal
                                  ? (UI_RECT_WIDTH(scrollBar->e.bounds) - UI_RECT_WIDTH(element->bounds))
                                  : (UI_RECT_HEIGHT(scrollBar->e.bounds) - UI_RECT_HEIGHT(element->bounds));
        scrollBar->position = (double)thumbPosition / size * (scrollBar->maximum - scrollBar->page);
        UIElementRefresh(&scrollBar->e);
        UIElementMessage(scrollBar->e.parent, UI_MSG_SCROLLED, 0, 0);
    } else if (message == UI_MSG_LEFT_UP) {
        scrollBar->inDrag = false;
    }

    return 0;
}


//


UIScrollBar *UIScrollBarCreate(UIElement *parent, uint32_t flags)
{
    UIScrollBar *scrollBar  = (UIScrollBar *)UIElementCreate(sizeof(UIScrollBar), parent, flags,
                                                             _UIScrollBarMessage, "Scroll Bar");
    bool         horizontal = scrollBar->horizontal = flags & UI_SCROLL_BAR_HORIZONTAL;
    UIElementCreate(sizeof(UIElement), &scrollBar->e, flags, _UIScrollUpDownMessage,
                    !horizontal ? "Scroll Up" : "Scroll Left")
        ->cp = (void *)(uintptr_t)0;
    UIElementCreate(sizeof(UIElement), &scrollBar->e, flags, _UIScrollThumbMessage, "Scroll Thumb");
    UIElementCreate(sizeof(UIElement), &scrollBar->e, flags, _UIScrollUpDownMessage,
                    !horizontal ? "Scroll Down" : "Scroll Right")
        ->cp = (void *)(uintptr_t)1;
    return scrollBar;
}
