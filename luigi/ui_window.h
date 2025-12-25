#ifndef LUIGI_WINDOW_H_
#define LUIGI_WINDOW_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "platform.h"
#include "ui_element.h"
#include "ui_painter.h"
#include "ui_shortcut.h"


typedef struct UIMenu UIMenu;


#define UI_WINDOW_MENU            (1 << 0)
#define UI_WINDOW_INSPECTOR       (1 << 1)
#define UI_WINDOW_CENTER_IN_OWNER (1 << 2)
#define UI_WINDOW_MAXIMIZE        (1 << 3)


extern const int UI_KEYCODE_A;
extern const int UI_KEYCODE_BACKSPACE;
extern const int UI_KEYCODE_DELETE;
extern const int UI_KEYCODE_DOWN;
extern const int UI_KEYCODE_END;
extern const int UI_KEYCODE_ENTER;
extern const int UI_KEYCODE_ESCAPE;
extern const int UI_KEYCODE_F1;
extern const int UI_KEYCODE_HOME;
extern const int UI_KEYCODE_LEFT;
extern const int UI_KEYCODE_RIGHT;
extern const int UI_KEYCODE_SPACE;
extern const int UI_KEYCODE_TAB;
extern const int UI_KEYCODE_UP;
extern const int UI_KEYCODE_INSERT;
extern const int UI_KEYCODE_0;
extern const int UI_KEYCODE_BACKTICK;
extern const int UI_KEYCODE_PAGE_DOWN;
extern const int UI_KEYCODE_PAGE_UP;


typedef struct UIWindow {
    UIElement  e;
    UIElement *dialog;

    UIShortcut *shortcuts;
    size_t      shortcutCount, shortcutAllocated;

    float scale;

    uint32_t        *bits;
    int              width, height;
    struct UIWindow *next;

    UIElement *hovered, *pressed, *focused, *dialogOldFocus;
    int        pressedButton;

    int cursorX, cursorY;
    int cursorStyle;

    // Set when a textbox is modified.
    // Useful for tracking whether changes to the loaded document have been saved.
    bool textboxModifiedFlag;

    bool ctrl, shift, alt;

    UIRectangle updateRegion;

#ifdef UI_DEBUG
    float lastFullFillCount;
#endif

    UI_PlatformWindow window;
} UIWindow;


//


UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int _width,
                         int _height);
int       UIMessageLoop(void);

void UIMenuShow(UIMenu *menu);


//


UIWindow *_UIFindWindow(Window window);
void      _UIWindowAdd(UIWindow *window);


void _UIWindowSetCursor(UIWindow *window, int cursor);
void _UIWindowSetPressed(UIWindow *window, UIElement *element, int button);

void _UIWindowGetScreenPosition(UIWindow *window, int *_x, int *_y);
void _UIWindowEndPaint(UIWindow *window, UIPainter *painter);

int  _UIWindowMessageCommon(UIElement *element, UIMessage message, int di, void *dp);
void _UIWindowDestroyCommon(UIWindow *window);


bool _UIMessageLoopSingle(int *result);

#ifdef __cplusplus
}
#endif


#endif // LUIGI_WINDOW_H_
