#ifndef LUIGI_PANEL_H_
#define LUIGI_PANEL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


typedef struct UIPanel {
#define UI_PANEL_HORIZONTAL     (1 << 0)
#define UI_PANEL_COLOR_1        (1 << 2)
#define UI_PANEL_COLOR_2        (1 << 3)
#define UI_PANEL_SMALL_SPACING  (1 << 5)
#define UI_PANEL_MEDIUM_SPACING (1 << 6)
#define UI_PANEL_LARGE_SPACING  (1 << 7)
#define UI_PANEL_SCROLL         (1 << 8)
#define UI_PANEL_EXPAND         (1 << 9)
    UIElement           e;
    struct UIScrollBar *scrollBar;
    UIRectangle         border;
    int                 gap;
} UIPanel;


typedef struct UIWrapPanel {
    UIElement e;
} UIWrapPanel;


//

UIPanel     *UIPanelCreate(UIElement *parent, uint32_t flags);
UIWrapPanel *UIWrapPanelCreate(UIElement *parent, uint32_t flags);


int _UIPanelLayout(UIPanel *panel, UIRectangle bounds, bool measure);
int _UIPanelMessage(UIElement *element, UIMessage message, int di, void *dp);
int _UIPanelCalculatePerFill(UIPanel *panel, int *_count, int hSpace, int vSpace, float scale);
int _UIPanelMeasure(UIPanel *panel, int di);

int  _UIWrapPanelMessage(UIElement *element, UIMessage message, int di, void *dp);
void _UIWrapPanelLayoutRow(UIWrapPanel *panel, uint32_t rowStart, uint32_t rowEnd, int rowY,
                           int rowHeight);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PANEL_H_
