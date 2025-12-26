#ifndef LUIGI_EVENT_H_
#define LUIGI_EVENT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_window.h"
#include <stdbool.h>


#define UI_UPDATE_HOVERED  (1)
#define UI_UPDATE_PRESSED  (2)
#define UI_UPDATE_FOCUSED  (3)
#define UI_UPDATE_DISABLED (4)


typedef enum UIMessage {
    // General messages.
    UI_MSG_PAINT,            // dp = pointer to UIPainter
    UI_MSG_PAINT_FOREGROUND, // after children have painted
    UI_MSG_LAYOUT,
    UI_MSG_DESTROY,
    UI_MSG_DEALLOCATE,
    UI_MSG_UPDATE, // di = UI_UPDATE_... constant
    UI_MSG_ANIMATE,
    UI_MSG_SCROLLED,
    UI_MSG_GET_WIDTH,           // di = height (if known); return width
    UI_MSG_GET_HEIGHT,          // di = width (if known); return height
    UI_MSG_GET_CHILD_STABILITY, // dp = child element; return stable axes, 1 (width) | 2 (height)

    // Input events.
    UI_MSG_INPUT_EVENTS_START, // not sent to disabled elements
    UI_MSG_LEFT_DOWN,
    UI_MSG_LEFT_UP,
    UI_MSG_MIDDLE_DOWN,
    UI_MSG_MIDDLE_UP,
    UI_MSG_RIGHT_DOWN,
    UI_MSG_RIGHT_UP,
    UI_MSG_KEY_TYPED,    // dp = pointer to UIKeyTyped; return 1 if handled
    UI_MSG_KEY_RELEASED, // dp = pointer to UIKeyTyped; return 1 if handled
    UI_MSG_MOUSE_MOVE,
    UI_MSG_MOUSE_DRAG,
    UI_MSG_MOUSE_WHEEL, // di = delta; return 1 if handled
    UI_MSG_CLICKED,
    UI_MSG_GET_CURSOR,         // return cursor code
    UI_MSG_PRESSED_DESCENDENT, // dp = pointer to child that is/contains pressed element
    UI_MSG_INPUT_EVENTS_END,

    // Specific elements.
    UI_MSG_VALUE_CHANGED,         // sent to notify that the element's value has changed
    UI_MSG_TABLE_GET_ITEM,        // dp = pointer to UITableGetItem; return string length
    UI_MSG_CODE_GET_MARGIN_COLOR, // di = line index (starts at 1); return color
    UI_MSG_CODE_DECORATE_LINE,    // dp = pointer to UICodeDecorateLine
    UI_MSG_TAB_SELECTED,          // sent to the tab that was selected (not the tab pane itself)

    // Windows.
    UI_MSG_WINDOW_DROP_FILES, // di = count, dp = char ** of paths
    UI_MSG_WINDOW_ACTIVATE,
    UI_MSG_WINDOW_CLOSE, // return 1 to prevent default (process exit for UIWindow; close for
                         // UIMDIChild)
    UI_MSG_WINDOW_UPDATE_START,
    UI_MSG_WINDOW_UPDATE_BEFORE_DESTROY,
    UI_MSG_WINDOW_UPDATE_BEFORE_LAYOUT,
    UI_MSG_WINDOW_UPDATE_BEFORE_PAINT,
    UI_MSG_WINDOW_UPDATE_END,

    // User-defined messages.
    UI_MSG_USER,
} UIMessage;


#ifdef __cplusplus
}
#endif


#endif // LUIGI_EVENT_H_
