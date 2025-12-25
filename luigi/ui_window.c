#include "ui_window.h"
#include "font.h"
#include "inspector.h"
#include "ui.h"
#include "ui_animation.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_menu.h"
#include "ui_painter.h"
#include "ui_theme.h"
#include "utils.h"


//


// ---------------------------
// ---------------------------
// ---------------------------


void _UIWindowEndPaint(UIWindow *window, UIPainter *painter)
{
    (void)painter;

    XPutImage(ui.display, window->window, DefaultGC(ui.display, 0), window->image,
              UI_RECT_TOP_LEFT(window->updateRegion), UI_RECT_TOP_LEFT(window->updateRegion),
              UI_RECT_SIZE(window->updateRegion));
}

void _UIWindowGetScreenPosition(UIWindow *window, int *_x, int *_y)
{
    Window child;
    XTranslateCoordinates(ui.display, window->window, DefaultRootWindow(ui.display), 0, 0, _x, _y,
                          &child);
}


/////////////////////////////////////////
// Common platform layer functionality.
/////////////////////////////////////////


void _UIWindowSetPressed(UIWindow *window, UIElement *element, int button)
{
    UIElement *previous   = window->pressed;
    window->pressed       = element;
    window->pressedButton = button;
    if (previous)
        UIElementMessage(previous, UI_MSG_UPDATE, UI_UPDATE_PRESSED, 0);
    if (element)
        UIElementMessage(element, UI_MSG_UPDATE, UI_UPDATE_PRESSED, 0);

    UIElement *ancestor = element;
    UIElement *child    = NULL;

    while (ancestor) {
        UIElementMessage(ancestor, UI_MSG_PRESSED_DESCENDENT, 0, child);
        child    = ancestor;
        ancestor = ancestor->parent;
    }
}


void _UIWindowDestroyCommon(UIWindow *window)
{
    UI_FREE(window->bits);
    UI_FREE(window->shortcuts);
}

void _UIInitialiseCommon(void)
{
    ui.theme = uiThemeClassic;

#ifdef UI_FREETYPE
    FT_Init_FreeType(&ui.ft);
    UIFontActivate(UIFontCreate(_UI_TO_STRING_2(UI_FONT_PATH), 11));
#else
    UIFontActivate(UIFontCreate(0, 0));
#endif
}

void _UIWindowAdd(UIWindow *window)
{
    window->scale    = 1.0f;
    window->e.window = window;
    window->hovered  = &window->e;
    window->next     = ui.windows;
    ui.windows       = window;
}


int _UIWindowMessageCommon(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_LAYOUT && element->childCount) {
        UIElementMove(element->children[0], element->bounds, false);
        if (element->window->dialog)
            UIElementMove(element->window->dialog, element->bounds, false);
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_GET_CHILD_STABILITY) {
        return 3; // Both width and height of the child element are ignored.
    }

    return 0;
}


int UIMessageLoop(void)
{
    _UIInspectorCreate();
    _UIUpdate();
#ifdef UI_AUTOMATION_TESTS
    return UIAutomationRunTests();
#else
    int result = 0;
    while (!ui.quit && _UIMessageLoopSingle(&result))
        ui.dialogResult = NULL;
    return result;
#endif
}
