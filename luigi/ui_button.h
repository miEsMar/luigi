#ifndef LUIGI_BUTTON_H_
#define LUIGI_BUTTON_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


#define UI_SIZE_BUTTON_MINIMUM_WIDTH (100)
#define UI_SIZE_BUTTON_PADDING       (16)
#define UI_SIZE_BUTTON_HEIGHT        (27)
#define UI_SIZE_BUTTON_CHECKED_AREA  (4)


#define UI_BUTTON_SMALL     (1 << 0)
#define UI_BUTTON_MENU_ITEM (1 << 1)
#define UI_BUTTON_CAN_FOCUS (1 << 2)
#define UI_BUTTON_DROP_DOWN (1 << 3)
#define UI_BUTTON_CHECKED   (1 << 15)


typedef struct UIButton {
    UIElement e;
    char     *label;
    ptrdiff_t labelBytes;
    void (*invoke)(void *cp);
} UIButton;


//


UIButton *UIButtonCreate(UIElement *parent, uint32_t flags, const char *label,
                         ptrdiff_t labelBytes);
void      UIButtonSetLabel(UIButton *button, const char *string, ptrdiff_t stringBytes);


int _UIButtonMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_BUTTON_H_
