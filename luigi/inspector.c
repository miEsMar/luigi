#include "inspector.h"
#include "ui.h"
#include "ui_code.h"
#include "ui_draw.h"
#include "ui_element.h"
#include "ui_event.h"
#include "ui_pane.h"
#include "ui_table.h"
#include "ui_window.h"
#include <stdarg.h>
#include <stdio.h>


//


void UIInspectorLog(const char *cFormat, ...)
{
    va_list arguments;
    va_start(arguments, cFormat);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), cFormat, arguments);
    UICodeInsertContent(ui.inspectorLog, buffer, -1, false);
    va_end(arguments);
    UIElementRefresh(&ui.inspectorLog->e);
}


UIElement *_UIInspectorFindNthElement(UIElement *element, int *index, int *depth)
{
    if (*index == 0) {
        return element;
    }

    *index = *index - 1;

    for (uint32_t i = 0; i < element->childCount; i++) {
        UIElement *child = element->children[i];

        if (!(child->flags & (UI_ELEMENT_DESTROY | UI_ELEMENT_HIDE))) {
            UIElement *result = _UIInspectorFindNthElement(child, index, depth);

            if (result) {
                if (depth) {
                    *depth = *depth + 1;
                }

                return result;
            }
        }
    }

    return NULL;
}


int _UIInspectorTableMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (!ui.inspectorTarget) {
        return 0;
    }

    if (message == UI_MSG_TABLE_GET_ITEM) {
        UITableGetItem *m     = (UITableGetItem *)dp;
        int             index = m->index;
        int             depth = 0;
        UIElement *element    = _UIInspectorFindNthElement(&ui.inspectorTarget->e, &index, &depth);
        if (!element)
            return 0;

        if (m->column == 0) {
            return snprintf(m->buffer, m->bufferBytes, "%.*s%s", depth * 2, "                ",
                            element->cClassName);
        } else if (m->column == 1) {
            return snprintf(m->buffer, m->bufferBytes, "%d:%d, %d:%d",
                            UI_RECT_ALL(element->bounds));
        } else if (m->column == 2) {
            return snprintf(m->buffer, m->bufferBytes, "%d%c", element->id,
                            element->window->focused == element ? '*' : ' ');
        }
    } else if (message == UI_MSG_MOUSE_MOVE) {
        int index =
            UITableHitTest(ui.inspectorTable, element->window->cursorX, element->window->cursorY);
        UIElement *element = NULL;
        if (index >= 0)
            element = _UIInspectorFindNthElement(&ui.inspectorTarget->e, &index, NULL);
        UIWindow *window     = ui.inspectorTarget;
        UIPainter painter    = {0};
        window->updateRegion = window->e.bounds;
        painter.bits         = window->bits;
        painter.width        = window->width;
        painter.height       = window->height;
        painter.clip         = UI_RECT_2S(window->width, window->height);

        for (int i = 0; i < window->width * window->height; i++) {
            window->bits[i] = 0xFF00FF;
        }

        _UIElementPaint(&window->e, &painter);
        painter.clip = UI_RECT_2S(window->width, window->height);

        if (element) {
            UIDrawInvert(&painter, element->bounds);
            UIDrawInvert(&painter, UIRectangleAdd(element->bounds, UI_RECT_1I(4)));
        }

        _UIWindowEndPaint(window, &painter);
    }

    return 0;
}


void _UIInspectorCreate(void)
{
    ui.inspector                     = UIWindowCreate(0, UI_WINDOW_INSPECTOR, "Inspector", 0, 0);
    UISplitPane *splitPane           = UISplitPaneCreate(&ui.inspector->e, 0, 0.5f);
    ui.inspectorTable                = UITableCreate(&splitPane->e, 0, "Class\tBounds\tID");
    ui.inspectorTable->e.messageUser = _UIInspectorTableMessage;
    ui.inspectorLog                  = UICodeCreate(&splitPane->e, 0);
}


int _UIInspectorCountElements(UIElement *element)
{
    int count = 1;

    for (uint32_t i = 0; i < element->childCount; i++) {
        UIElement *child = element->children[i];

        if (!(child->flags & (UI_ELEMENT_DESTROY | UI_ELEMENT_HIDE))) {
            count += _UIInspectorCountElements(child);
        }
    }

    return count;
}


void _UIInspectorRefresh(void)
{
    if (!ui.inspectorTarget || !ui.inspector || !ui.inspectorTable)
        return;
    ui.inspectorTable->itemCount = _UIInspectorCountElements(&ui.inspectorTarget->e);
    UITableResizeColumns(ui.inspectorTable);
    UIElementRefresh(&ui.inspectorTable->e);
}


void _UIInspectorSetFocusedWindow(UIWindow *window)
{
    if (!ui.inspector || !ui.inspectorTable)
        return;

    if (window->e.flags & UI_WINDOW_INSPECTOR) {
        return;
    }

    if (ui.inspectorTarget != window) {
        ui.inspectorTarget = window;
        _UIInspectorRefresh();
    }
}

// void _UIInspectorCreate() {}
// void _UIInspectorSetFocusedWindow(UIWindow *window) {}
// void _UIInspectorRefresh() {}
