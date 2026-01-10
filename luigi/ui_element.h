#ifndef LUIGI_ELEMENT_H_
#define LUIGI_ELEMENT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_painter.h"
#include "ui_rect.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Forward declare
typedef enum UIMessage UIMessage;


#define UI_ELEMENT_V_FILL (1 << 16)
#define UI_ELEMENT_H_FILL (1 << 17)
#define UI_ELEMENT_FILL   (UI_ELEMENT_V_FILL | UI_ELEMENT_H_FILL)

#define UI_ELEMENT_WINDOW      (1 << 18)
#define UI_ELEMENT_PARENT_PUSH (1 << 19)
#define UI_ELEMENT_TAB_STOP    (1 << 20)
#define UI_ELEMENT_NON_CLIENT                                                                      \
    (1 << 21) // Don't destroy in UIElementDestroyDescendents, like scroll bars.
#define UI_ELEMENT_DISABLED (1 << 22) // Don't receive input events.
#define UI_ELEMENT_BORDER   (1 << 23)

#define UI_ELEMENT_HIDE                (1 << 27)
#define UI_ELEMENT_RELAYOUT            (1 << 28)
#define UI_ELEMENT_RELAYOUT_DESCENDENT (1 << 29)
#define UI_ELEMENT_DESTROY             (1 << 30)
#define UI_ELEMENT_DESTROY_DESCENDENT  (1 << 31)


#define UI_ELEMENT_INVISIBLE(e) ((e)->flags & (UI_ELEMENT_HIDE | UI_ELEMENT_DISABLED))
#define UI_ELEMENT_VISIBLE(e)   !UI_ELEMENT_INVISIBLE(e)


typedef struct UIElement {
    uint32_t flags; // First 16 bits are element specific.
    uint32_t id;
    uint32_t childCount;
    uint32_t _unused0;

    struct UIElement  *parent;
    struct UIElement **children;
    struct UIWindow   *window;

    UIRectangle bounds, clip;

    void *cp; // Context pointer (for user).

    int (*messageClass)(struct UIElement *element, UIMessage message, int di /* data integer */,
                        void *dp /* data pointer */);
    int (*messageUser)(struct UIElement *element, UIMessage message, int di, void *dp);

    const char *cClassName;
} UIElement;


//


UIElement *UIElementCreate(size_t bytes, UIElement *parent, uint32_t flags,
                           int (*message)(UIElement *, UIMessage, int, void *),
                           const char *cClassName);

UIElement *UIElementFindByPoint(UIElement *element, int x, int y);

void       UIElementMove(UIElement *element, UIRectangle bounds, bool layout);
void       UIElementRepaint(UIElement *element, UIRectangle *region);
void       UIElementRelayout(UIElement *element);
void       UIElementMeasurementsChanged(UIElement *element, int which);
UIElement *UIElementChangeParent(UIElement *element, UIElement *newParent, UIElement *insertBefore);
int        UIElementMessage(UIElement *element, UIMessage message, int di, void *dp);

UIRectangle UIElementScreenBounds(UIElement *element);

void UIElementRefresh(UIElement *element);

void UIElementFocus(UIElement *element);


UIElement *UIParentPush(UIElement *element);
UIElement *UIParentPop(void);

void UIElementDestroy(UIElement *element);
void UIElementDestroyDescendents(UIElement *element);


//


UIElement *_UIElementNextOrPreviousSibling(UIElement *element, bool previous);
void       _UIElementPaint(UIElement *element, UIPainter *painter);
void       _UIElementDestroyDescendents(UIElement *element, bool topLevel);
bool       _UIDestroy(UIElement *element);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_ELEMENT_H_
