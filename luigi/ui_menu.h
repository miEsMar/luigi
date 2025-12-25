#ifndef LUIGI_MENU_H_
#define LUIGI_MENU_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"
#include "ui_scroll.h"
#include "ui_window.h"


#define UI_SIZE_MENU_ITEM_HEIGHT        (24)
#define UI_SIZE_MENU_ITEM_MINIMUM_WIDTH (160)
#define UI_SIZE_MENU_ITEM_MARGIN        (9)

#define UI_MENU_PLACE_ABOVE (1 << 0)
#define UI_MENU_NO_SCROLL   (1 << 1)
#if defined(UI_COCOA)
typedef NSMenu UIMenu;
#elif defined(UI_ESSENCE)
typedef EsMenu UIMenu;
#else
typedef struct UIMenu {
    UIElement    e;
    int          pointX, pointY;
    UIScrollBar *vScroll;
    UIWindow    *parentWindow;
} UIMenu;
#endif


//


bool UIMenusOpen(void);


#if !defined(UI_ESSENCE) && !defined(UI_COCOA)
UIMenu *UIMenuCreate(UIElement *parent, uint32_t flags);
void    UIMenuAddItem(UIMenu *menu, uint32_t flags, const char *label, ptrdiff_t labelBytes,
                      void (*invoke)(void *cp), void *cp);


void _UIMenuPrepare(UIMenu *menu, int *width, int *height);
int  _UIMenuItemMessage(UIElement *element, UIMessage message, int di, void *dp);
int  _UIMenuMessage(UIElement *element, UIMessage message, int di, void *dp);
bool _UIMenusClose(void);
#endif


#ifdef __cplusplus
}
#endif

#endif // LUIGI_MENU_H_
