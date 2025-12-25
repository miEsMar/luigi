#include "ui.h"
#include "ui_animation.h"
#include "ui_element.h"
#include "ui_event.h"
#include "utils.h"


struct Luigi ui;


void _UIInitialiseCommon(void)
{
    ui.theme = uiThemeDark;

#ifdef UI_FREETYPE
    FT_Init_FreeType(&ui.ft);
    UIFontActivate(UIFontCreate(_UI_TO_STRING_2(UI_FONT_PATH), 11));
#else
    UIFontActivate(UIFontCreate(0, 0));
#endif
}


void _UIUpdate(void)
{
    UIWindow  *window = ui.windows;
    UIWindow **link   = &ui.windows;

    while (window) {
        UIWindow *next = window->next;

        UIElementMessage(&window->e, UI_MSG_WINDOW_UPDATE_START, 0, 0);
        UIElementMessage(&window->e, UI_MSG_WINDOW_UPDATE_BEFORE_DESTROY, 0, 0);

        if (_UIDestroy(&window->e)) {
            *link = next;
        } else {
            link = &window->next;

            UIElementMessage(&window->e, UI_MSG_WINDOW_UPDATE_BEFORE_LAYOUT, 0, 0);
            UIElementMove(&window->e, window->e.bounds, false);
            UIElementMessage(&window->e, UI_MSG_WINDOW_UPDATE_BEFORE_PAINT, 0, 0);

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
                _UIElementPaint(&window->e, &painter);
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


bool _UIDestroy(UIElement *element)
{
    if (element->flags & UI_ELEMENT_DESTROY_DESCENDENT) {
        element->flags &= ~UI_ELEMENT_DESTROY_DESCENDENT;

        for (uintptr_t i = 0; i < element->childCount; i++) {
            if (_UIDestroy(element->children[i])) {
                UI_MEMMOVE(&element->children[i], &element->children[i + 1],
                           sizeof(UIElement *) * (element->childCount - i - 1));
                element->childCount--, i--;
            }
        }
    }

    if (element->flags & UI_ELEMENT_DESTROY) {
        UIElementMessage(element, UI_MSG_DEALLOCATE, 0, 0);

        if (element->window->pressed == element) {
            _UIWindowSetPressed(element->window, NULL, 0);
        }

        if (element->window->hovered == element) {
            element->window->hovered = &element->window->e;
        }

        if (element->window->focused == element) {
            element->window->focused = NULL;
        }

        if (element->window->dialogOldFocus == element) {
            element->window->dialogOldFocus = NULL;
        }

        UIElementAnimate(element, true);
        UI_FREE(element->children);
        UI_FREE(element);
        return true;
    } else {
        return false;
    }
}
