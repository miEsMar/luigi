#include "ui_menu.h"
#include "ui.h"
#include "ui_button.h"
#include "ui_draw.h"
#include "ui_element.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_rect.h"
#include "ui_scroll.h"
#include "ui_window.h"


//


static int _UIMenuMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIMenu *menu = (UIMenu *)element;

    if (message == UI_MSG_GET_WIDTH) {
        int width = 0;

        for (uint32_t i = 0; i < element->childCount; i++) {
            UIElement *child = element->children[i];

            if (~child->flags & UI_ELEMENT_NON_CLIENT) {
                int w = UIElementMessage(child, UI_MSG_GET_WIDTH, 0, 0);
                if (w > width)
                    width = w;
            }
        }

        return width + 4 + UI_SIZE_SCROLL_BAR;
    } else if (message == UI_MSG_GET_HEIGHT) {
        int height = 0;

        for (uint32_t i = 0; i < element->childCount; i++) {
            UIElement *child = element->children[i];

            if (~child->flags & UI_ELEMENT_NON_CLIENT) {
                height += UIElementMessage(child, UI_MSG_GET_HEIGHT, 0, 0);
            }
        }

        return height + 4;
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds, UI_DRAW_CONTROL_MENU, NULL, 0, 0,
                      element->window->scale);
    } else if (message == UI_MSG_LAYOUT) {
        int position      = element->bounds.t + 2 - menu->vScroll->position;
        int totalHeight   = 0;
        int scrollBarSize = (menu->e.flags & UI_MENU_NO_SCROLL) ? 0 : UI_SIZE_SCROLL_BAR;

        for (uint32_t i = 0; i < element->childCount; i++) {
            UIElement *child = element->children[i];

            if (~child->flags & UI_ELEMENT_NON_CLIENT) {
                int height = UIElementMessage(child, UI_MSG_GET_HEIGHT, 0, 0);
                UIElementMove(child,
                              UI_RECT_4(element->bounds.l + 2,
                                        element->bounds.r - scrollBarSize - 2, position,
                                        position + height),
                              false);
                position += height;
                totalHeight += height;
            }
        }

        UIRectangle scrollBarBounds = element->bounds;
        scrollBarBounds.l           = scrollBarBounds.r - scrollBarSize * element->window->scale;
        menu->vScroll->maximum      = totalHeight;
        menu->vScroll->page         = UI_RECT_HEIGHT(element->bounds);
        UIElementMove(&menu->vScroll->e, scrollBarBounds, true);
    } else if (message == UI_MSG_KEY_TYPED) {
        UIKeyTyped *m = (UIKeyTyped *)dp;

        if (m->code == UI_KEYCODE_ESCAPE) {
            _UIMenusClose();
            return 1;
        }
    } else if (message == UI_MSG_MOUSE_WHEEL) {
        return UIElementMessage(&menu->vScroll->e, message, di, dp);
    } else if (message == UI_MSG_SCROLLED) {
        UIElementRefresh(element);
    }

    return 0;
}


//


bool UIMenusOpen(void)
{
    UIWindow *window = ui.windows;

    while (window) {
        if (window->e.flags & UI_WINDOW_MENU) {
            return true;
        }
        window = window->next;
    }
    return false;
}


/////////////////////////////////////////
// Menus (common).
/////////////////////////////////////////


bool _UIMenusClose(void)
{
    UIWindow *window    = ui.windows;
    bool      anyClosed = false;

    while (window) {
        if (window->e.flags & UI_WINDOW_MENU) {
            UIElementDestroy(&window->e);
            anyClosed = true;
        }

        window = window->next;
    }

    return anyClosed;
}

#if !defined(UI_ESSENCE) && !defined(UI_COCOA)
int _UIMenuItemMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_CLICKED) {
        _UIMenusClose();
    }

    return 0;
}


void UIMenuAddItem(UIMenu *menu, uint32_t flags, const char *label, ptrdiff_t labelBytes,
                   void (*invoke)(void *cp), void *cp)
{
    UIButton *button = UIButtonCreate(&menu->e, flags | UI_BUTTON_MENU_ITEM, label, labelBytes);
    button->invoke   = invoke;
    button->e.messageUser = _UIMenuItemMessage;
    button->e.cp          = cp;
}


void _UIMenuPrepare(UIMenu *menu, int *width, int *height)
{
    *width  = UIElementMessage(&menu->e, UI_MSG_GET_WIDTH, 0, 0);
    *height = UIElementMessage(&menu->e, UI_MSG_GET_HEIGHT, 0, 0);

    if (menu->e.flags & UI_MENU_PLACE_ABOVE) {
        menu->pointY -= *height;
    }
}


UIMenu *UIMenuCreate(UIElement *parent, uint32_t flags)
{
    UIWindow *window = Luigi_CreateWindow(parent->window, UI_WINDOW_MENU, 0, 0, 0);
    UIMenu   *menu =
        (UIMenu *)UIElementCreate(sizeof(UIMenu), &window->e, flags, _UIMenuMessage, "Menu");
    menu->vScroll      = UIScrollBarCreate(&menu->e, UI_ELEMENT_NON_CLIENT);
    menu->parentWindow = parent->window;

    if (parent->parent) {
        UIRectangle screenBounds = UIElementScreenBounds(parent);
        menu->pointX             = screenBounds.l;
        menu->pointY = (flags & UI_MENU_PLACE_ABOVE) ? (screenBounds.t + 1) : (screenBounds.b - 1);
    } else {
        int x = 0, y = 0;
        Luigi_Platform_get_screen_pos(&parent->window->window, &x, &y);

        menu->pointX = parent->window->cursorX + x;
        menu->pointY = parent->window->cursorY + y;
    }

    return menu;
}
#endif
