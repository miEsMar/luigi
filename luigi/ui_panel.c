#include "ui_panel.h"
#include "ui.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_pane.h"
#include "ui_scroll.h"


//


static int _UIPanelMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIPanel *panel      = (UIPanel *)element;
    bool     horizontal = element->flags & UI_PANEL_HORIZONTAL;

    if (message == UI_MSG_LAYOUT) {
        int scrollBarWidth = panel->scrollBar ? (UI_SIZE_SCROLL_BAR * element->window->scale) : 0;
        UIRectangle bounds = element->bounds;
        bounds.r -= scrollBarWidth;

        if (panel->scrollBar) {
            UIRectangle scrollBarBounds = element->bounds;
            scrollBarBounds.l           = scrollBarBounds.r - scrollBarWidth;
            panel->scrollBar->maximum   = _UIPanelLayout(panel, bounds, true);
            panel->scrollBar->page      = UI_RECT_HEIGHT(element->bounds);
            UIElementMove(&panel->scrollBar->e, scrollBarBounds, true);
        }

        _UIPanelLayout(panel, bounds, false);
    } else if (message == UI_MSG_GET_WIDTH) {
        if (horizontal) {
            return _UIPanelLayout(panel, UI_RECT_4(0, 0, 0, di), true);
        } else {
            return _UIPanelMeasure(panel, di);
        }
    } else if (message == UI_MSG_GET_HEIGHT) {
        if (horizontal) {
            return _UIPanelMeasure(panel, di);
        } else {
            int width =
                di && panel->scrollBar ? (di - UI_SIZE_SCROLL_BAR * element->window->scale) : di;
            return _UIPanelLayout(panel, UI_RECT_4(0, width, 0, 0), true);
        }
    } else if (message == UI_MSG_PAINT) {
        if (element->flags & UI_PANEL_COLOR_1) {
            UIDrawBlock((UIPainter *)dp, element->bounds, ui.theme.panel1);
        } else if (element->flags & UI_PANEL_COLOR_2) {
            UIDrawBlock((UIPainter *)dp, element->bounds, ui.theme.panel2);
        }
    } else if (message == UI_MSG_MOUSE_WHEEL && panel->scrollBar) {
        return UIElementMessage(&panel->scrollBar->e, message, di, dp);
    } else if (message == UI_MSG_SCROLLED) {
        UIElementRefresh(element);
    } else if (message == UI_MSG_GET_CHILD_STABILITY) {
        UIElement *child = (UIElement *)dp;
        return ((element->flags & UI_PANEL_EXPAND) ? (horizontal ? 2 : 1) : 0) |
               ((child->flags & UI_ELEMENT_H_FILL) ? 1 : 0) |
               ((child->flags & UI_ELEMENT_V_FILL) ? 2 : 0);
    }

    return 0;
}


static int _UIWrapPanelMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIWrapPanel *panel = (UIWrapPanel *)element;

    if (message == UI_MSG_LAYOUT || message == UI_MSG_GET_HEIGHT) {
        int totalHeight = 0;
        int rowPosition = 0;
        int rowHeight   = 0;
        int rowLimit    = message == UI_MSG_LAYOUT ? UI_RECT_WIDTH(element->bounds) : di;

        uint32_t rowStart = 0;

        for (uint32_t i = 0; i < panel->e.childCount; i++) {
            UIElement *child = panel->e.children[i];
            if (child->flags & UI_ELEMENT_HIDE)
                continue;

            int height = UIElementMessage(child, UI_MSG_GET_HEIGHT, 0, 0);
            int width  = UIElementMessage(child, UI_MSG_GET_WIDTH, 0, 0);

            if (rowLimit && rowPosition + width > rowLimit) {
                _UIWrapPanelLayoutRow(panel, rowStart, i, totalHeight, rowHeight);
                totalHeight += rowHeight;
                rowPosition = rowHeight = 0;
                rowStart                = i;
            }

            if (height > rowHeight) {
                rowHeight = height;
            }

            rowPosition += width;
        }

        if (message == UI_MSG_GET_HEIGHT) {
            return totalHeight + rowHeight;
        } else {
            _UIWrapPanelLayoutRow(panel, rowStart, panel->e.childCount, totalHeight, rowHeight);
        }
    }

    return 0;
}


//


int _UIPanelCalculatePerFill(UIPanel *panel, int *_count, int hSpace, int vSpace, float scale)
{
    bool horizontal = panel->e.flags & UI_PANEL_HORIZONTAL;
    int  available  = horizontal ? hSpace : vSpace;
    int  count = 0, fill = 0, perFill = 0;

    for (uint32_t i = 0; i < panel->e.childCount; i++) {
        UIElement *child = panel->e.children[i];

        if (child->flags & (UI_ELEMENT_HIDE | UI_ELEMENT_NON_CLIENT)) {
            continue;
        }

        count++;

        if (horizontal) {
            if (child->flags & UI_ELEMENT_H_FILL) {
                fill++;
            } else if (available > 0) {
                available -= UIElementMessage(child, UI_MSG_GET_WIDTH, vSpace, 0);
            }
        } else {
            if (child->flags & UI_ELEMENT_V_FILL) {
                fill++;
            } else if (available > 0) {
                available -= UIElementMessage(child, UI_MSG_GET_HEIGHT, hSpace, 0);
            }
        }
    }

    if (count) {
        available -= (count - 1) * (int)(panel->gap * scale);
    }

    if (available > 0 && fill) {
        perFill = available / fill;
    }

    if (_count) {
        *_count = count;
    }

    return perFill;
}


int _UIPanelMeasure(UIPanel *panel, int di)
{
    bool horizontal = panel->e.flags & UI_PANEL_HORIZONTAL;
    int  perFill = _UIPanelCalculatePerFill(panel, NULL, horizontal ? di : 0, horizontal ? 0 : di,
                                            panel->e.window->scale);
    int  size    = 0;

    for (uint32_t i = 0; i < panel->e.childCount; i++) {
        UIElement *child = panel->e.children[i];
        if (child->flags & (UI_ELEMENT_HIDE | UI_ELEMENT_NON_CLIENT))
            continue;
        int childSize = UIElementMessage(
            child, horizontal ? UI_MSG_GET_HEIGHT : UI_MSG_GET_WIDTH,
            (child->flags & (horizontal ? UI_ELEMENT_H_FILL : UI_ELEMENT_V_FILL)) ? perFill : 0, 0);
        if (childSize > size)
            size = childSize;
    }

    int border =
        horizontal ? (panel->border.t + panel->border.b) : (panel->border.l + panel->border.r);
    return size + border * panel->e.window->scale;
}


int _UIPanelLayout(UIPanel *panel, UIRectangle bounds, bool measure)
{
    bool  horizontal = panel->e.flags & UI_PANEL_HORIZONTAL;
    float scale      = panel->e.window->scale;
    int   position   = (horizontal ? panel->border.l : panel->border.t) * scale;
    if (panel->scrollBar && !measure)
        position -= panel->scrollBar->position;
    int  hSpace        = UI_RECT_WIDTH(bounds) - UI_RECT_TOTAL_H(panel->border) * scale;
    int  vSpace        = UI_RECT_HEIGHT(bounds) - UI_RECT_TOTAL_V(panel->border) * scale;
    int  count         = 0;
    int  perFill       = _UIPanelCalculatePerFill(panel, &count, hSpace, vSpace, scale);
    int  scaledBorder2 = (horizontal ? panel->border.t : panel->border.l) * panel->e.window->scale;
    bool expand        = panel->e.flags & UI_PANEL_EXPAND;

    for (uint32_t i = 0; i < panel->e.childCount; i++) {
        UIElement *child = panel->e.children[i];

        if (child->flags & (UI_ELEMENT_HIDE | UI_ELEMENT_NON_CLIENT)) {
            continue;
        }

        if (horizontal) {
            int height =
                ((child->flags & UI_ELEMENT_V_FILL) || expand)
                    ? vSpace
                    : UIElementMessage(child, UI_MSG_GET_HEIGHT,
                                       (child->flags & UI_ELEMENT_H_FILL) ? perFill : 0, 0);
            int         width = (child->flags & UI_ELEMENT_H_FILL)
                                    ? perFill
                                    : UIElementMessage(child, UI_MSG_GET_WIDTH, height, 0);
            UIRectangle relative =
                UI_RECT_4(position, position + width, scaledBorder2 + (vSpace - height) / 2,
                          scaledBorder2 + (vSpace + height) / 2);
            if (!measure)
                UIElementMove(child, UIRectangleTranslate(relative, bounds), false);
            position += width + panel->gap * scale;
        } else {
            int         width  = ((child->flags & UI_ELEMENT_H_FILL) || expand)
                                     ? hSpace
                                     : UIElementMessage(child, UI_MSG_GET_WIDTH,
                                               (child->flags & UI_ELEMENT_V_FILL) ? perFill : 0, 0);
            int         height = (child->flags & UI_ELEMENT_V_FILL)
                                     ? perFill
                                     : UIElementMessage(child, UI_MSG_GET_HEIGHT, width, 0);
            UIRectangle relative =
                UI_RECT_4(scaledBorder2 + (hSpace - width) / 2,
                          scaledBorder2 + (hSpace + width) / 2, position, position + height);
            if (!measure)
                UIElementMove(child, UIRectangleTranslate(relative, bounds), false);
            position += height + panel->gap * scale;
        }
    }

    return position - (count ? panel->gap : 0) * scale +
           (horizontal ? panel->border.r : panel->border.b) * scale;
}


UIPanel *UIPanelCreate(UIElement *parent, uint32_t flags)
{
    UIPanel *panel =
        (UIPanel *)UIElementCreate(sizeof(UIPanel), parent, flags, _UIPanelMessage, "Panel");

    if (flags & UI_PANEL_LARGE_SPACING) {
        panel->border = UI_RECT_1(UI_SIZE_PANE_LARGE_BORDER);
        panel->gap    = UI_SIZE_PANE_LARGE_GAP;
    } else if (flags & UI_PANEL_MEDIUM_SPACING) {
        panel->border = UI_RECT_1(UI_SIZE_PANE_MEDIUM_BORDER);
        panel->gap    = UI_SIZE_PANE_MEDIUM_GAP;
    } else if (flags & UI_PANEL_SMALL_SPACING) {
        panel->border = UI_RECT_1(UI_SIZE_PANE_SMALL_BORDER);
        panel->gap    = UI_SIZE_PANE_SMALL_GAP;
    }

    if (flags & UI_PANEL_SCROLL) {
        panel->scrollBar = UIScrollBarCreate(&panel->e, UI_ELEMENT_NON_CLIENT);
    }

    return panel;
}

void _UIWrapPanelLayoutRow(UIWrapPanel *panel, uint32_t rowStart, uint32_t rowEnd, int rowY,
                           int rowHeight)
{
    int rowPosition = 0;

    for (uint32_t i = rowStart; i < rowEnd; i++) {
        UIElement *child = panel->e.children[i];
        if (child->flags & UI_ELEMENT_HIDE)
            continue;
        int         height = UIElementMessage(child, UI_MSG_GET_HEIGHT, 0, 0);
        int         width  = UIElementMessage(child, UI_MSG_GET_WIDTH, 0, 0);
        UIRectangle relative =
            UI_RECT_4(rowPosition, rowPosition + width, rowY + rowHeight / 2 - height / 2,
                      rowY + rowHeight / 2 + height / 2);
        UIElementMove(child, UIRectangleTranslate(relative, panel->e.bounds), false);
        rowPosition += width;
    }
}


UIWrapPanel *UIWrapPanelCreate(UIElement *parent, uint32_t flags)
{
    return (UIWrapPanel *)UIElementCreate(sizeof(UIWrapPanel), parent, flags, _UIWrapPanelMessage,
                                          "Wrap Panel");
}
