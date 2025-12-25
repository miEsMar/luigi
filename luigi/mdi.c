#include "mdi.h"
#include "ui.h"
#include "ui_button.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_rect.h"
#include "ui_window.h"
#include "utils.h"


//


static inline int _UIMDIChildHitTest(UIMDIChild *mdiChild, int x, int y)
{
    UIElement *element = &mdiChild->e;
    UI_MDI_CHILD_CALCULATE_LAYOUT(element->bounds, element->window->scale);
    int cornerSize = UI_SIZE_MDI_CHILD_CORNER * element->window->scale;
    if (!UIRectangleContains(element->bounds, x, y) || UIRectangleContains(content, x, y))
        return -1;
    else if (x < element->bounds.l + cornerSize && y < element->bounds.t + cornerSize)
        return 0b1010;
    else if (x > element->bounds.r - cornerSize && y < element->bounds.t + cornerSize)
        return 0b0110;
    else if (x < element->bounds.l + cornerSize && y > element->bounds.b - cornerSize)
        return 0b1001;
    else if (x > element->bounds.r - cornerSize && y > element->bounds.b - cornerSize)
        return 0b0101;
    else if (x < element->bounds.l + borderSize)
        return 0b1000;
    else if (x > element->bounds.r - borderSize)
        return 0b0100;
    else if (y < element->bounds.t + borderSize)
        return 0b0010;
    else if (y > element->bounds.b - borderSize)
        return 0b0001;
    else if (UIRectangleContains(title, x, y))
        return 0b1111;
    else
        return -1;
}


static int _UIMDIChildMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIMDIChild *mdiChild = (UIMDIChild *)element;

    if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds, UI_DRAW_CONTROL_MDI_CHILD, mdiChild->title,
                      mdiChild->titleBytes, 0, element->window->scale);
    } else if (message == UI_MSG_GET_WIDTH) {
        UIElement *child = element->childCount ? element->children[element->childCount - 1] : NULL;
        int        width = 2 * UI_SIZE_MDI_CHILD_BORDER;
        width +=
            (child ? UIElementMessage(
                         child, message,
                         di ? (di - UI_SIZE_MDI_CHILD_TITLE + UI_SIZE_MDI_CHILD_BORDER) : 0, dp)
                   : 0);
        if (width < UI_SIZE_MDI_CHILD_MINIMUM_WIDTH)
            width = UI_SIZE_MDI_CHILD_MINIMUM_WIDTH;
        return width;
    } else if (message == UI_MSG_GET_HEIGHT) {
        UIElement *child  = element->childCount ? element->children[element->childCount - 1] : NULL;
        int        height = UI_SIZE_MDI_CHILD_TITLE + UI_SIZE_MDI_CHILD_BORDER;
        height += (child ? UIElementMessage(child, message,
                                            di ? (di - 2 * UI_SIZE_MDI_CHILD_BORDER) : 0, dp)
                         : 0);
        if (height < UI_SIZE_MDI_CHILD_MINIMUM_HEIGHT)
            height = UI_SIZE_MDI_CHILD_MINIMUM_HEIGHT;
        return height;
    } else if (message == UI_MSG_LAYOUT) {
        UI_MDI_CHILD_CALCULATE_LAYOUT(element->bounds, element->window->scale);

        int position = title.r;

        for (uint32_t i = 0; i < element->childCount - 1; i++) {
            UIElement *child = element->children[i];
            int        width = UIElementMessage(child, UI_MSG_GET_WIDTH, 0, 0);
            UIElementMove(child, UI_RECT_4(position - width, position, title.t, title.b), false);
            position -= width;
        }

        UIElement *child = element->childCount ? element->children[element->childCount - 1] : NULL;

        if (child) {
            UIElementMove(child, content, false);
        }
    } else if (message == UI_MSG_GET_CURSOR) {
        int hitTest =
            _UIMDIChildHitTest(mdiChild, element->window->cursorX, element->window->cursorY);
        if (hitTest == 0b1000)
            return UI_CURSOR_RESIZE_LEFT;
        if (hitTest == 0b0010)
            return UI_CURSOR_RESIZE_UP;
        if (hitTest == 0b0110)
            return UI_CURSOR_RESIZE_UP_RIGHT;
        if (hitTest == 0b1010)
            return UI_CURSOR_RESIZE_UP_LEFT;
        if (hitTest == 0b0100)
            return UI_CURSOR_RESIZE_RIGHT;
        if (hitTest == 0b0001)
            return UI_CURSOR_RESIZE_DOWN;
        if (hitTest == 0b1001)
            return UI_CURSOR_RESIZE_DOWN_LEFT;
        if (hitTest == 0b0101)
            return UI_CURSOR_RESIZE_DOWN_RIGHT;
        return UI_CURSOR_ARROW;
    } else if (message == UI_MSG_LEFT_DOWN) {
        mdiChild->dragHitTest =
            _UIMDIChildHitTest(mdiChild, element->window->cursorX, element->window->cursorY);
        mdiChild->dragOffset = UIRectangleAdd(
            element->bounds, UI_RECT_2(-element->window->cursorX, -element->window->cursorY));
    } else if (message == UI_MSG_LEFT_UP) {
        if (mdiChild->bounds.l < 0)
            mdiChild->bounds.r -= mdiChild->bounds.l, mdiChild->bounds.l = 0;
        if (mdiChild->bounds.t < 0)
            mdiChild->bounds.b -= mdiChild->bounds.t, mdiChild->bounds.t = 0;
        UIElementRefresh(element->parent);
    } else if (message == UI_MSG_MOUSE_DRAG) {
        if (mdiChild->dragHitTest > 0) {
#define _UI_MDI_CHILD_MOVE_EDGE(bit, edge, cursor, size, opposite, negate, minimum, offset)        \
    if (mdiChild->dragHitTest & bit)                                                               \
        mdiChild->bounds.edge =                                                                    \
            mdiChild->dragOffset.edge + element->window->cursor - element->parent->bounds.offset;  \
    if ((mdiChild->dragHitTest & bit) && size(mdiChild->bounds) < minimum)                         \
        mdiChild->bounds.edge = mdiChild->bounds.opposite negate minimum;
            _UI_MDI_CHILD_MOVE_EDGE(0b1000, l, cursorX, UI_RECT_WIDTH, r, -,
                                    UI_SIZE_MDI_CHILD_MINIMUM_WIDTH, l);
            _UI_MDI_CHILD_MOVE_EDGE(0b0100, r, cursorX, UI_RECT_WIDTH, l, +,
                                    UI_SIZE_MDI_CHILD_MINIMUM_WIDTH, l);
            _UI_MDI_CHILD_MOVE_EDGE(0b0010, t, cursorY, UI_RECT_HEIGHT, b, -,
                                    UI_SIZE_MDI_CHILD_MINIMUM_HEIGHT, t);
            _UI_MDI_CHILD_MOVE_EDGE(0b0001, b, cursorY, UI_RECT_HEIGHT, t, +,
                                    UI_SIZE_MDI_CHILD_MINIMUM_HEIGHT, t);
            UIElementRefresh(element->parent);
        }
    } else if (message == UI_MSG_DESTROY) {
        UIMDIClient *client = (UIMDIClient *)element->parent;

        if (client->active == mdiChild) {
            client->active = (UIMDIChild *)(client->e.childCount == 1
                                                ? NULL
                                                : client->e.children[client->e.childCount - 2]);
        }
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(mdiChild->title);
    }

    return 0;
}


static inline int _UIMDIClientMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIMDIClient *client = (UIMDIClient *)element;

    if (message == UI_MSG_PAINT) {
        if (~element->flags & UI_MDI_CLIENT_TRANSPARENT) {
            UIDrawBlock((UIPainter *)dp, element->bounds, ui.theme.panel2);
        }
    } else if (message == UI_MSG_LAYOUT) {
        const UIRectangle zero_rect = UI_RECT_1(0);
        for (uint32_t i = 0; i < element->childCount; i++) {
            UIMDIChild *mdiChild = (UIMDIChild *)element->children[i];
            UI_ASSERT(mdiChild->e.messageClass == _UIMDIChildMessage);

            if (UIRectangleEquals(mdiChild->bounds, zero_rect)) {
                int width  = UIElementMessage(&mdiChild->e, UI_MSG_GET_WIDTH, 0, 0);
                int height = UIElementMessage(&mdiChild->e, UI_MSG_GET_HEIGHT, width, 0);
                if (client->cascade + width > element->bounds.r ||
                    client->cascade + height > element->bounds.b)
                    client->cascade = 0;
                mdiChild->bounds = UI_RECT_4(client->cascade, client->cascade + width,
                                             client->cascade, client->cascade + height);
                client->cascade += UI_SIZE_MDI_CASCADE * element->window->scale;
            }

            UIRectangle bounds =
                UIRectangleAdd(mdiChild->bounds, UI_RECT_2(element->bounds.l, element->bounds.t));
            UIElementMove(&mdiChild->e, bounds, false);
        }
    } else if (message == UI_MSG_PRESSED_DESCENDENT) {
        UIMDIChild *child = (UIMDIChild *)dp;

        if (child && child != client->active) {
            for (uint32_t i = 0; i < element->childCount; i++) {
                if (element->children[i] == &child->e) {
                    UI_MEMMOVE(&element->children[i], &element->children[i + 1],
                               sizeof(UIElement *) * (element->childCount - i - 1));
                    element->children[element->childCount - 1] = &child->e;
                    break;
                }
            }

            client->active = child;
            UIElementRefresh(element);
        }
    }

    return 0;
}


static inline void _UIMDIChildCloseButton(void *_child)
{
    UIElement *child = (UIElement *)_child;

    if (!UIElementMessage(child, UI_MSG_WINDOW_CLOSE, 0, 0)) {
        UIElementDestroy(child);
        UIElementRefresh(child->parent);
    }
}


//


UIMDIChild *UIMDIChildCreate(UIElement *parent, uint32_t flags, UIRectangle initialBounds,
                             const char *title, ptrdiff_t titleBytes)
{
    UI_ASSERT(parent->messageClass == _UIMDIClientMessage);

    UIMDIChild  *mdiChild  = (UIMDIChild *)UIElementCreate(sizeof(UIMDIChild), parent, flags,
                                                           _UIMDIChildMessage, "MDIChild");
    UIMDIClient *mdiClient = (UIMDIClient *)parent;

    mdiChild->bounds  = initialBounds;
    mdiChild->title   = UIStringCopy(title, (mdiChild->titleBytes = titleBytes));
    mdiClient->active = mdiChild;

    if (flags & UI_MDI_CHILD_CLOSE_BUTTON) {
        UIButton *closeButton =
            UIButtonCreate(&mdiChild->e, UI_BUTTON_SMALL | UI_ELEMENT_NON_CLIENT, "X", 1);
        closeButton->invoke = _UIMDIChildCloseButton;
        closeButton->e.cp   = mdiChild;
    }

    return mdiChild;
}

UIMDIClient *UIMDIClientCreate(UIElement *parent, uint32_t flags)
{
    return (UIMDIClient *)UIElementCreate(sizeof(UIMDIClient), parent, flags, _UIMDIClientMessage,
                                          "MDIClient");
}
