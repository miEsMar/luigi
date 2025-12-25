#ifndef LUIGI_TABLE_H_
#define LUIGI_TABLE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"
#include "ui_scroll.h"
#include <stdbool.h>
#include <stddef.h>


#define UI_SIZE_TABLE_HEADER     (26)
#define UI_SIZE_TABLE_COLUMN_GAP (20)
#define UI_SIZE_TABLE_ROW        (20)


typedef struct UITableGetItem {
    char  *buffer;
    size_t bufferBytes;
    int    index, column;
    bool   isSelected;
} UITableGetItem;


typedef struct UITable {
    UIElement    e;
    UIScrollBar *vScroll, *hScroll;
    int          itemCount;
    char        *columns;
    int         *columnWidths, columnCount, columnHighlight;
} UITable;


//

UITable *UITableCreate(UIElement *parent, uint32_t flags, const char *columns);


bool UITableEnsureVisible(UITable *table, int index);
int  UITableHeaderHitTest(UITable *table, int x, int y);
int  UITableHitTest(UITable *table, int x, int y);
void UITableResizeColumns(UITable *table);


int _UITableMessage(UIElement *element, UIMessage message, int di, void *dp);

#ifdef __cplusplus
}
#endif

#endif // LUIGI_TABLE_H_
