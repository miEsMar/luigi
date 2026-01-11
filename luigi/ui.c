#include "ui.h"
#include "font.h"
#include "inspector.h"
#include "ui_element.h"
#include "ui_event.h"
#include "ui_window.h"
#include "utils.h"


struct Luigi ui;


void Luigi_Init(Luigi_InitConfig *config)
{
    // struct Luigi *luigi = UI_CALLOC(sizeof(*luigi));
    // if (NULL == luigi) {
    //     goto ret_;
    // }


    ui.platform = Luigi_PlatformInit();

    ui.theme = uiThemeDark;

#ifdef UI_FREETYPE
    FT_Init_FreeType(&ui.ft);
    UIFontActivate(UIFontCreate(_UI_TO_STRING_2(UI_FONT_PATH), 11));
#else
    UI_FONT_LOAD_DEFAULT();
#endif

    if (config->with_inspector) {
        Luigi_InspectorCreate();
    }

    return;
}


int Luigi_Loop(void)
{
    Luigi_UpdateUI();

#ifdef UI_AUTOMATION_TESTS
    return UIAutomationRunTests();
#else
    int result = 0;
    while (!ui.quit && _UIMessageLoopSingle(&result)) {
        ui.dialogResult = NULL;
    }
    return result;
#endif
}


//


void Luigi_UpdateUI(void)
{
    UIWindow  *window = ui.windows;
    UIWindow **link   = &ui.windows;

    while (window) {
        UIElement *e    = &window->e;
        UIWindow  *next = window->next;

        UIElementMessage(e, UI_MSG_WINDOW_UPDATE_START, 0, 0);
        UIElementMessage(e, UI_MSG_WINDOW_UPDATE_BEFORE_DESTROY, 0, 0);

        if (Luigi_ElementDestroy(e)) {
            *link = next;
        } else {
            link = &window->next;

            UIElementMessage(e, UI_MSG_WINDOW_UPDATE_BEFORE_LAYOUT, 0, 0);
            UIElementMove(e, window->e.bounds, false);
            UIElementMessage(e, UI_MSG_WINDOW_UPDATE_BEFORE_PAINT, 0, 0);

            if (UI_RECT_VALID(window->updateRegion)) {
#ifdef __cplusplus
                UIPainter painter = {};
#else
                UIPainter painter = {0};
#endif
                painter.bits   = window->bits;
                painter.width  = window->width;
                painter.height = window->height;
                painter.clip   = UIRectangleIntersection(UI_RECT_2S(window->width, window->height),
                                                         window->updateRegion);
                Luigi_ElementPaint(&window->e, &painter);
                _UIWindowEndPaint(window, &painter);
                window->updateRegion = UI_RECT_1(0);

#ifdef UI_DEBUG
                window->lastFullFillCount =
                    (float)painter.fillCount /
                    (UI_RECT_WIDTH(window->updateRegion) * UI_RECT_HEIGHT(window->updateRegion));
#endif
            }

            UIElementMessage(&window->e, UI_MSG_WINDOW_UPDATE_END, 0, 0);
        }

        window = next;
    }
}
