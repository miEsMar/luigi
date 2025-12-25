#ifndef LUIGI_PANE_H_
#define LUIGI_PANE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_button.h"
#include "ui_element.h"
#include "ui_panel.h"


#define UI_SIZE_TAB_PANE_SPACE_TOP  (2)
#define UI_SIZE_TAB_PANE_SPACE_LEFT (4)

#define UI_SIZE_SPLITTER (8)

#define UI_SIZE_PANE_LARGE_BORDER  (20)
#define UI_SIZE_PANE_LARGE_GAP     (10)
#define UI_SIZE_PANE_MEDIUM_BORDER (5)
#define UI_SIZE_PANE_MEDIUM_GAP    (5)
#define UI_SIZE_PANE_SMALL_BORDER  (3)
#define UI_SIZE_PANE_SMALL_GAP     (3)


typedef struct UISplitPane {
#define UI_SPLIT_PANE_VERTICAL (1 << 0)
    UIElement e;
    float     weight;
} UISplitPane;


typedef struct UITabPane {
    UIElement e;
    char     *tabs;
    uint32_t  active;
} UITabPane;


typedef struct UIExpandPane {
    UIElement e;
    UIButton *button;
    UIPanel  *panel;
    bool      expanded;
} UIExpandPane;


//


UISplitPane *UISplitPaneCreate(UIElement *parent, uint32_t flags, float weight);
UITabPane   *UITabPaneCreate(UIElement *parent, uint32_t flags, const char *tabs);


int _UISplitterMessage(UIElement *element, UIMessage message, int di, void *dp);
int _UISplitPaneMessage(UIElement *element, UIMessage message, int di, void *dp);
int _UITabPaneMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif

#endif // LUIGI_PANE_H_
