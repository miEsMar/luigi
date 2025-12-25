#include "ui_event.h"
#include "ui.h"
#include "ui_menu.h"
#include "ui_window.h"


bool _UIWindowInputEvent(UIWindow *window, UIMessage message, int di, void *dp)
{
    bool handled = true;

    if (window->pressed) {
        if (message == UI_MSG_MOUSE_MOVE) {
            UIElementMessage(window->pressed, UI_MSG_MOUSE_DRAG, di, dp);
        } else if (message == UI_MSG_LEFT_UP && window->pressedButton == 1) {
            if (window->hovered == window->pressed) {
                UIElementMessage(window->pressed, UI_MSG_CLICKED, di, dp);
                if (ui.quit || ui.dialogResult)
                    goto end;
            }

            if (window->pressed) {
                UIElementMessage(window->pressed, UI_MSG_LEFT_UP, di, dp);
                if (ui.quit || ui.dialogResult)
                    goto end;
                _UIWindowSetPressed(window, NULL, 1);
            }
        } else if (message == UI_MSG_MIDDLE_UP && window->pressedButton == 2) {
            UIElementMessage(window->pressed, UI_MSG_MIDDLE_UP, di, dp);
            if (ui.quit || ui.dialogResult)
                goto end;
            _UIWindowSetPressed(window, NULL, 2);
        } else if (message == UI_MSG_RIGHT_UP && window->pressedButton == 3) {
            UIElementMessage(window->pressed, UI_MSG_RIGHT_UP, di, dp);
            if (ui.quit || ui.dialogResult)
                goto end;
            _UIWindowSetPressed(window, NULL, 3);
        }
    }

    if (window->pressed) {
        bool inside = UIRectangleContains(window->pressed->clip, window->cursorX, window->cursorY);

        if (inside && window->hovered == &window->e) {
            window->hovered = window->pressed;
            UIElementMessage(window->pressed, UI_MSG_UPDATE, UI_UPDATE_HOVERED, 0);
        } else if (!inside && window->hovered == window->pressed) {
            window->hovered = &window->e;
            UIElementMessage(window->pressed, UI_MSG_UPDATE, UI_UPDATE_HOVERED, 0);
        }

        if (ui.quit || ui.dialogResult)
            goto end;
    }

    if (!window->pressed) {
        UIElement *hovered = UIElementFindByPoint(&window->e, window->cursorX, window->cursorY);

        if (message == UI_MSG_MOUSE_MOVE) {
            UIElementMessage(hovered, UI_MSG_MOUSE_MOVE, di, dp);

            int cursor = UIElementMessage(window->hovered, UI_MSG_GET_CURSOR, di, dp);

            if (cursor != window->cursorStyle) {
                window->cursorStyle = cursor;
                _UIWindowSetCursor(window, cursor);
            }
        } else if (message == UI_MSG_LEFT_DOWN) {
            if ((window->e.flags & UI_WINDOW_MENU) || !_UIMenusClose()) {
                _UIWindowSetPressed(window, hovered, 1);
                UIElementMessage(hovered, UI_MSG_LEFT_DOWN, di, dp);
            }
        } else if (message == UI_MSG_MIDDLE_DOWN) {
            if ((window->e.flags & UI_WINDOW_MENU) || !_UIMenusClose()) {
                _UIWindowSetPressed(window, hovered, 2);
                UIElementMessage(hovered, UI_MSG_MIDDLE_DOWN, di, dp);
            }
        } else if (message == UI_MSG_RIGHT_DOWN) {
            if ((window->e.flags & UI_WINDOW_MENU) || !_UIMenusClose()) {
                _UIWindowSetPressed(window, hovered, 3);
                UIElementMessage(hovered, UI_MSG_RIGHT_DOWN, di, dp);
            }
        } else if (message == UI_MSG_MOUSE_WHEEL) {
            UIElement *element = hovered;

            while (element) {
                if (UIElementMessage(element, UI_MSG_MOUSE_WHEEL, di, dp)) {
                    break;
                }

                element = element->parent;
            }
        } else if (message == UI_MSG_KEY_TYPED || message == UI_MSG_KEY_RELEASED) {
            handled = false;

            if (window->focused) {
                UIElement *element = window->focused;

                while (element) {
                    if (UIElementMessage(element, message, di, dp)) {
                        handled = true;
                        break;
                    }

                    element = element->parent;
                }
            } else {
                if (UIElementMessage(&window->e, message, di, dp)) {
                    handled = true;
                }
            }

            if (!handled && !UIMenusOpen() && message == UI_MSG_KEY_TYPED) {
                UIKeyTyped *m = (UIKeyTyped *)dp;

                if (m->code == UI_KEYCODE_TAB && !window->ctrl && !window->alt) {
                    UIElement *start   = window->focused ? window->focused : &window->e;
                    UIElement *element = start;

                    do {
                        if (element->childCount &&
                            !(element->flags & (UI_ELEMENT_HIDE | UI_ELEMENT_DISABLED))) {
                            element = window->shift ? element->children[element->childCount - 1]
                                                    : element->children[0];
                            continue;
                        }

                        while (element) {
                            UIElement *sibling =
                                _UIElementNextOrPreviousSibling(element, window->shift);
                            if (sibling) {
                                element = sibling;
                                break;
                            }
                            element = element->parent;
                        }

                        if (!element) {
                            element = &window->e;
                        }
                    } while (element != start &&
                             ((~element->flags & UI_ELEMENT_TAB_STOP) ||
                              (element->flags & (UI_ELEMENT_HIDE | UI_ELEMENT_DISABLED))));

                    if (~element->flags & UI_ELEMENT_WINDOW) {
                        UIElementFocus(element);
                    }

                    handled = true;
                } else if (!window->dialog) {
                    for (intptr_t i = window->shortcutCount - 1; i >= 0; i--) {
                        UIShortcut *shortcut = window->shortcuts + i;

                        if (shortcut->code == m->code && shortcut->ctrl == window->ctrl &&
                            shortcut->shift == window->shift && shortcut->alt == window->alt) {
                            shortcut->invoke(shortcut->cp);
                            handled = true;
                            break;
                        }
                    }
                } else if (window->dialog) {
                    UIElementMessage(window->dialog, message, di, dp);
                }
            }
        }

        if (ui.quit || ui.dialogResult)
            goto end;

        if (hovered != window->hovered) {
            UIElement *previous = window->hovered;
            window->hovered     = hovered;
            UIElementMessage(previous, UI_MSG_UPDATE, UI_UPDATE_HOVERED, 0);
            UIElementMessage(window->hovered, UI_MSG_UPDATE, UI_UPDATE_HOVERED, 0);
        }
    }

end:
    _UIUpdate();
    return handled;
}
