#include "ui_table.h"
#include "ui.h"
#include "ui_draw.h"
#include "ui_element.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_window.h"
#include "utils.h"


//


static int _UITableMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UITable *table = (UITable *)element;

    if (message == UI_MSG_PAINT) {
        UIPainter  *painter = (UIPainter *)dp;
        UIRectangle bounds  = element->bounds;
        bounds.r            = table->vScroll->e.bounds.l;
        UIDrawControl(painter, element->bounds,
                      UI_DRAW_CONTROL_TABLE_BACKGROUND |
                          UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      NULL, 0, 0, element->window->scale);
        char           buffer[256];
        UIRectangle    row       = bounds;
        int            rowHeight = UI_SIZE_TABLE_ROW * element->window->scale;
        UITableGetItem m         = {0};
        m.buffer                 = buffer;
        m.bufferBytes            = sizeof(buffer);
        row.t += UI_SIZE_TABLE_HEADER * table->e.window->scale;
        row.t -= (int64_t)table->vScroll->position % rowHeight;
        int hovered = UITableHitTest(table, element->window->cursorX, element->window->cursorY);
        UIRectangle oldClip = painter->clip;
        painter->clip       = UIRectangleIntersection(
            oldClip,
            UI_RECT_4(bounds.l, bounds.r,
                            bounds.t + (int)(UI_SIZE_TABLE_HEADER * element->window->scale), bounds.b));

        for (int i = table->vScroll->position / rowHeight; i < table->itemCount; i++) {
            if (row.t > painter->clip.b) {
                break;
            }

            row.b        = row.t + rowHeight;
            m.index      = i;
            m.isSelected = false;
            m.column     = 0;
            int bytes    = UIElementMessage(element, UI_MSG_TABLE_GET_ITEM, 0, &m);

            uint32_t rowFlags = (m.isSelected ? UI_DRAW_CONTROL_STATE_SELECTED : 0) |
                                (hovered == i ? UI_DRAW_CONTROL_STATE_HOVERED : 0);
            UIDrawControl(painter, row, UI_DRAW_CONTROL_TABLE_ROW | rowFlags, NULL, 0, 0,
                          element->window->scale);

            UIRectangle cell = row;
            cell.l += UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale -
                      (int64_t)table->hScroll->position;

            for (int j = 0; j < table->columnCount; j++) {
                if (j) {
                    m.column = j;
                    bytes    = UIElementMessage(element, UI_MSG_TABLE_GET_ITEM, 0, &m);
                }

                cell.r = cell.l + table->columnWidths[j];
                if ((size_t)bytes > m.bufferBytes && bytes > 0)
                    bytes = m.bufferBytes;
                UIDrawControl(painter, cell, UI_DRAW_CONTROL_TABLE_CELL | rowFlags, buffer, bytes,
                              0, element->window->scale);
                cell.l +=
                    table->columnWidths[j] + UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale;
            }

            row.t += rowHeight;
        }

        bounds        = element->bounds;
        painter->clip = UIRectangleIntersection(oldClip, bounds);
        if (table->hScroll)
            bounds.l -= (int64_t)table->hScroll->position;

        UIRectangle header = bounds;
        header.b           = header.t + UI_SIZE_TABLE_HEADER * table->e.window->scale;
        header.l += UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale;

        int position = 0;
        int index    = 0;

        if (table->columnCount) {
            while (true) {
                int end = position;
                for (; table->columns[end] != '\t' && table->columns[end]; end++)
                    ;

                header.r = header.l + table->columnWidths[index];
                UIDrawControl(
                    painter, header,
                    UI_DRAW_CONTROL_TABLE_HEADER |
                        (index == table->columnHighlight ? UI_DRAW_CONTROL_STATE_SELECTED : 0),
                    table->columns + position, end - position, 0, element->window->scale);
                header.l +=
                    table->columnWidths[index] + UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale;

                if (table->columns[end] == '\t') {
                    position = end + 1;
                    index++;
                } else {
                    break;
                }
            }
        }
    } else if (message == UI_MSG_LAYOUT) {
        int scrollBarSize = UI_SIZE_SCROLL_BAR * table->e.window->scale;
        int columnGap     = UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale;

        table->vScroll->maximum = table->itemCount * UI_SIZE_TABLE_ROW * element->window->scale;
        table->hScroll->maximum = columnGap;
        for (int i = 0; i < table->columnCount; i++) {
            table->hScroll->maximum += table->columnWidths[i] + columnGap;
        }

        int vSpace = table->vScroll->page =
            UI_RECT_HEIGHT(element->bounds) - UI_SIZE_TABLE_HEADER * element->window->scale;
        int hSpace = table->hScroll->page = UI_RECT_WIDTH(element->bounds);
        _UI_LAYOUT_SCROLL_BAR_PAIR(table);
    } else if (message == UI_MSG_MOUSE_MOVE || message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_SCROLLED) {
        UIElementRefresh(element);
    } else if (message == UI_MSG_MOUSE_WHEEL) {
        return UIElementMessage(&table->vScroll->e, message, di, dp);
    } else if (message == UI_MSG_LEFT_DOWN) {
        UIElementFocus(element);
    } else if (message == UI_MSG_KEY_TYPED) {
        UIKeyTyped *m = (UIKeyTyped *)dp;

        if ((m->code == UI_KEYCODE_UP || m->code == UI_KEYCODE_DOWN ||
             m->code == UI_KEYCODE_PAGE_UP || m->code == UI_KEYCODE_PAGE_DOWN ||
             m->code == UI_KEYCODE_HOME || m->code == UI_KEYCODE_END) &&
            !element->window->ctrl && !element->window->alt && !element->window->shift) {
            _UI_KEY_INPUT_VSCROLL(
                table, UI_SIZE_TABLE_ROW * element->window->scale,
                (float)(element->bounds.t - table->hScroll->e.bounds.t + UI_SIZE_TABLE_HEADER) * 4 /
                    5);
            return 1;
        } else if ((m->code == UI_KEYCODE_LEFT || m->code == UI_KEYCODE_RIGHT) &&
                   !element->window->ctrl && !element->window->alt && !element->window->shift) {
            table->hScroll->position +=
                m->code == UI_KEYCODE_LEFT ? -ui.activeFont->glyphWidth : ui.activeFont->glyphWidth;
            UIElementRefresh(&table->e);
            return 1;
        }
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(table->columns);
        UI_FREE(table->columnWidths);
    }

    return 0;
}


//


int UITableHitTest(UITable *table, int x, int y)
{
    x -= table->e.bounds.l;

    if (x < 0 || x >= table->vScroll->e.bounds.l) {
        return -1;
    }

    y -= (table->e.bounds.t + UI_SIZE_TABLE_HEADER * table->e.window->scale) -
         table->vScroll->position;

    int rowHeight = UI_SIZE_TABLE_ROW * table->e.window->scale;

    if (y < 0 || y >= rowHeight * table->itemCount) {
        return -1;
    }

    return y / rowHeight;
}


int UITableHeaderHitTest(UITable *table, int x, int y)
{
    if (!table->columnCount)
        return -1;
    UIRectangle header = table->e.bounds;
    header.b           = header.t + UI_SIZE_TABLE_HEADER * table->e.window->scale;
    header.l += UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale;
    int position = 0, index = 0;

    while (true) {
        int end = position;
        for (; table->columns[end] != '\t' && table->columns[end]; end++)
            ;
        header.r = header.l + table->columnWidths[index];
        if (UIRectangleContains(header, x, y))
            return index;
        header.l += table->columnWidths[index] + UI_SIZE_TABLE_COLUMN_GAP * table->e.window->scale;
        if (table->columns[end] != '\t')
            break;
        position = end + 1, index++;
    }

    return -1;
}


bool UITableEnsureVisible(UITable *table, int index)
{
    int rowHeight = UI_SIZE_TABLE_ROW * table->e.window->scale;
    int y         = index * rowHeight;
    y -= table->vScroll->position;
    int height =
        UI_RECT_HEIGHT(table->e.bounds) - UI_SIZE_TABLE_HEADER * table->e.window->scale - rowHeight;

    if (y < 0) {
        table->vScroll->position += y;
        UIElementRefresh(&table->e);
        return true;
    } else if (y > height) {
        table->vScroll->position -= height - y;
        UIElementRefresh(&table->e);
        return true;
    } else {
        return false;
    }
}


void UITableResizeColumns(UITable *table)
{
    int position = 0;
    int count    = 0;

    while (true) {
        int end = position;
        for (; table->columns[end] != '\t' && table->columns[end]; end++)
            ;
        count++;
        if (table->columns[end] == '\t')
            position = end + 1;
        else
            break;
    }

    UI_FREE(table->columnWidths);
    table->columnWidths = (int *)UI_MALLOC(count * sizeof(int));
    table->columnCount  = count;

    position = 0;

    char           buffer[256];
    UITableGetItem m = {0};
    m.buffer         = buffer;
    m.bufferBytes    = sizeof(buffer);

    while (true) {
        int end = position;
        for (; table->columns[end] != '\t' && table->columns[end]; end++)
            ;

        int longest = UIMeasureStringWidth(table->columns + position, end - position);

        for (int i = 0; i < table->itemCount; i++) {
            m.index   = i;
            int bytes = UIElementMessage(&table->e, UI_MSG_TABLE_GET_ITEM, 0, &m);
            int width = UIMeasureStringWidth(buffer, bytes);

            if (width > longest) {
                longest = width;
            }
        }

        table->columnWidths[m.column] = longest;
        m.column++;
        if (table->columns[end] == '\t')
            position = end + 1;
        else
            break;
    }

    UIElementRepaint(&table->e, NULL);
}


UITable *UITableCreate(UIElement *parent, uint32_t flags, const char *columns)
{
    UITable *table =
        (UITable *)UIElementCreate(sizeof(UITable), parent, flags, _UITableMessage, "Table");
    table->vScroll         = UIScrollBarCreate(&table->e, 0);
    table->hScroll         = UIScrollBarCreate(&table->e, UI_SCROLL_BAR_HORIZONTAL);
    table->columns         = UIStringCopy(columns, -1);
    table->columnHighlight = -1;
    return table;
}
