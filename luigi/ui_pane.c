#include "ui_pane.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_rect.h"
#include "ui_window.h"


/////////////////////////////////////////
// Split panes.
/////////////////////////////////////////


int _UISplitterMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UISplitPane *splitPane = (UISplitPane *)element->parent;
    bool         vertical  = splitPane->e.flags & UI_SPLIT_PANE_VERTICAL;

    if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_SPLITTER | (vertical ? UI_DRAW_CONTROL_STATE_VERTICAL : 0) |
                          UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      NULL, 0, 0, element->window->scale);
    } else if (message == UI_MSG_GET_CURSOR) {
        return vertical ? UI_CURSOR_SPLIT_V : UI_CURSOR_SPLIT_H;
    } else if (message == UI_MSG_MOUSE_DRAG) {
        int cursor       = vertical ? element->window->cursorY : element->window->cursorX;
        int splitterSize = UI_SIZE_SPLITTER * element->window->scale;
        int space =
            (vertical ? UI_RECT_HEIGHT(splitPane->e.bounds) : UI_RECT_WIDTH(splitPane->e.bounds)) -
            splitterSize;
        float oldWeight   = splitPane->weight;
        splitPane->weight = (float)(cursor - ((float)splitterSize / 2) -
                                    (vertical ? splitPane->e.bounds.t : splitPane->e.bounds.l)) /
                            space;
        if (splitPane->weight < 0.05f)
            splitPane->weight = 0.05f;
        if (splitPane->weight > 0.95f)
            splitPane->weight = 0.95f;

        if (splitPane->e.children[2]->messageClass == _UISplitPaneMessage &&
            (splitPane->e.children[2]->flags & UI_SPLIT_PANE_VERTICAL) ==
                (splitPane->e.flags & UI_SPLIT_PANE_VERTICAL)) {
            UISplitPane *subSplitPane = (UISplitPane *)splitPane->e.children[2];
            subSplitPane->weight      = (splitPane->weight - oldWeight - subSplitPane->weight +
                                    oldWeight * subSplitPane->weight) /
                                   (-1 + splitPane->weight);
            if (subSplitPane->weight < 0.05f)
                subSplitPane->weight = 0.05f;
            if (subSplitPane->weight > 0.95f)
                subSplitPane->weight = 0.95f;
        }

        UIElementRefresh(&splitPane->e);
    }

    return 0;
}


int _UISplitPaneMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UISplitPane *splitPane = (UISplitPane *)element;
    bool         vertical  = splitPane->e.flags & UI_SPLIT_PANE_VERTICAL;

    if (message == UI_MSG_LAYOUT) {
        UIElement *splitter = element->children[0];
        UIElement *left     = element->children[1];
        UIElement *right    = element->children[2];

        int splitterSize = UI_SIZE_SPLITTER * element->window->scale;
        int space = (vertical ? UI_RECT_HEIGHT(element->bounds) : UI_RECT_WIDTH(element->bounds)) -
                    splitterSize;
        int leftSize  = space * splitPane->weight;
        int rightSize = space - leftSize;

        if (vertical) {
            UIElementMove(left,
                          UI_RECT_4(element->bounds.l, element->bounds.r, element->bounds.t,
                                    element->bounds.t + leftSize),
                          false);
            UIElementMove(splitter,
                          UI_RECT_4(element->bounds.l, element->bounds.r,
                                    element->bounds.t + leftSize,
                                    element->bounds.t + leftSize + splitterSize),
                          false);
            UIElementMove(right,
                          UI_RECT_4(element->bounds.l, element->bounds.r,
                                    element->bounds.b - rightSize, element->bounds.b),
                          false);
        } else {
            UIElementMove(left,
                          UI_RECT_4(element->bounds.l, element->bounds.l + leftSize,
                                    element->bounds.t, element->bounds.b),
                          false);
            UIElementMove(splitter,
                          UI_RECT_4(element->bounds.l + leftSize,
                                    element->bounds.l + leftSize + splitterSize, element->bounds.t,
                                    element->bounds.b),
                          false);
            UIElementMove(right,
                          UI_RECT_4(element->bounds.r - rightSize, element->bounds.r,
                                    element->bounds.t, element->bounds.b),
                          false);
        }
    }

    return 0;
}


UISplitPane *UISplitPaneCreate(UIElement *parent, uint32_t flags, float weight)
{
    UISplitPane *splitPane = (UISplitPane *)UIElementCreate(sizeof(UISplitPane), parent, flags,
                                                            _UISplitPaneMessage, "Split Pane");
    splitPane->weight      = weight;
    UIElementCreate(sizeof(UIElement), &splitPane->e, 0, _UISplitterMessage, "Splitter");
    return splitPane;
}


/////////////////////////////////////////
// Tab panes.
/////////////////////////////////////////

int _UITabPaneMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UITabPane *tabPane = (UITabPane *)element;

    if (message == UI_MSG_PAINT) {
        UIPainter  *painter = (UIPainter *)dp;
        UIRectangle top     = element->bounds;
        top.b               = top.t + UI_SIZE_BUTTON_HEIGHT * element->window->scale;
        UIDrawControl(painter, top, UI_DRAW_CONTROL_TAB_BAND, NULL, 0, 0, element->window->scale);

        UIRectangle tab = top;
        tab.l += UI_SIZE_TAB_PANE_SPACE_LEFT * element->window->scale;
        tab.t += UI_SIZE_TAB_PANE_SPACE_TOP * element->window->scale;

        int      position = 0;
        uint32_t index    = 0;

        while (true) {
            int end = position;
            for (; tabPane->tabs[end] != '\t' && tabPane->tabs[end]; end++)
                ;

            int width = UIMeasureStringWidth(tabPane->tabs, end - position);
            tab.r     = tab.l + width + UI_SIZE_BUTTON_PADDING;

            UIDrawControl(painter, tab,
                          UI_DRAW_CONTROL_TAB |
                              (tabPane->active == index ? UI_DRAW_CONTROL_STATE_SELECTED : 0),
                          tabPane->tabs + position, end - position, 0, element->window->scale);
            tab.l = tab.r - 1;

            if (tabPane->tabs[end] == '\t') {
                position = end + 1;
                index++;
            } else {
                break;
            }
        }
    } else if (message == UI_MSG_LEFT_DOWN) {
        UIRectangle tab = element->bounds;
        tab.b           = tab.t + UI_SIZE_BUTTON_HEIGHT * element->window->scale;
        tab.l += UI_SIZE_TAB_PANE_SPACE_LEFT * element->window->scale;
        tab.t += UI_SIZE_TAB_PANE_SPACE_TOP * element->window->scale;

        int position = 0;
        int index    = 0;

        while (true) {
            int end = position;
            for (; tabPane->tabs[end] != '\t' && tabPane->tabs[end]; end++)
                ;

            int width = UIMeasureStringWidth(tabPane->tabs, end - position);
            tab.r     = tab.l + width + UI_SIZE_BUTTON_PADDING;

            if (UIRectangleContains(tab, element->window->cursorX, element->window->cursorY)) {
                tabPane->active = index;
                UIElementRelayout(element);
                UIElementRepaint(element, NULL);
                break;
            }

            tab.l = tab.r - 1;

            if (tabPane->tabs[end] == '\t') {
                position = end + 1;
                index++;
            } else {
                break;
            }
        }
    } else if (message == UI_MSG_LAYOUT) {
        UIRectangle content = element->bounds;
        content.t += UI_SIZE_BUTTON_HEIGHT * element->window->scale;

        for (uint32_t index = 0; index < element->childCount; index++) {
            UIElement *child = element->children[index];

            if (tabPane->active == index) {
                child->flags &= ~UI_ELEMENT_HIDE;
                UIElementMove(child, content, false);
                UIElementMessage(child, UI_MSG_TAB_SELECTED, 0, 0);
            } else {
                child->flags |= UI_ELEMENT_HIDE;
            }
        }
    } else if (message == UI_MSG_GET_HEIGHT) {
        int baseHeight = UI_SIZE_BUTTON_HEIGHT * element->window->scale;

        for (uint32_t index = 0; index < element->childCount; index++) {
            UIElement *child = element->children[index];

            if (tabPane->active == index) {
                return baseHeight + UIElementMessage(child, UI_MSG_GET_HEIGHT, di, dp);
            }
        }
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(tabPane->tabs);
    }

    return 0;
}


UITabPane *UITabPaneCreate(UIElement *parent, uint32_t flags, const char *tabs)
{
    UITabPane *tabPane = (UITabPane *)UIElementCreate(sizeof(UITabPane), parent, flags,
                                                      _UITabPaneMessage, "Tab Pane");
    tabPane->tabs      = UIStringCopy(tabs, -1);
    return tabPane;
}
