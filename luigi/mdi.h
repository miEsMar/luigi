#ifndef LUIGI_MDI_H_
#define LUIGI_MDI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"
#include "ui_rect.h"


#define UI_SIZE_MDI_CHILD_BORDER         (6)
#define UI_SIZE_MDI_CHILD_TITLE          (30)
#define UI_SIZE_MDI_CHILD_CORNER         (12)
#define UI_SIZE_MDI_CHILD_MINIMUM_WIDTH  (100)
#define UI_SIZE_MDI_CHILD_MINIMUM_HEIGHT (50)
#define UI_SIZE_MDI_CASCADE              (30)


#define UI_MDI_CHILD_CALCULATE_LAYOUT(bounds, scale)                                               \
    int         titleSize  = UI_SIZE_MDI_CHILD_TITLE * scale;                                      \
    int         borderSize = UI_SIZE_MDI_CHILD_BORDER * scale;                                     \
    UIRectangle title      = UIRectangleAdd(bounds, UI_RECT_4(borderSize, -borderSize, 0, 0));     \
    title.b                = title.t + titleSize;                                                  \
    UIRectangle content =                                                                          \
        UIRectangleAdd(bounds, UI_RECT_4(borderSize, -borderSize, titleSize, -borderSize));


typedef struct UIMDIClient {
#define UI_MDI_CLIENT_TRANSPARENT (1 << 0)
    UIElement          e;
    struct UIMDIChild *active;
    int                cascade;
} UIMDIClient;


typedef struct UIMDIChild {
#define UI_MDI_CHILD_CLOSE_BUTTON (1 << 0)
    UIElement   e;
    UIRectangle bounds;
    char       *title;
    ptrdiff_t   titleBytes;
    int         dragHitTest;
    UIRectangle dragOffset;
} UIMDIChild;


//


UIMDIClient *UIMDIClientCreate(UIElement *parent, uint32_t flags);
UIMDIChild  *UIMDIChildCreate(UIElement *parent, uint32_t flags, UIRectangle initialBounds,
                              const char *title, ptrdiff_t titleBytes);


int  _UIMDIClientMessage(UIElement *element, UIMessage message, int di, void *dp);
int  _UIMDIChildMessage(UIElement *element, UIMessage message, int di, void *dp);
int  _UIMDIChildHitTest(UIMDIChild *mdiChild, int x, int y);
void _UIMDIChildCloseButton(void *_child);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_MDI_H_
