#ifndef LUIGI_CHECKBOX_H_
#define LUIGI_CHECKBOX_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


#define UI_SIZE_CHECKBOX_BOX (14)
#define UI_SIZE_CHECKBOX_GAP (8)


typedef struct UICheckbox {
#define UI_CHECKBOX_ALLOW_INDETERMINATE (1 << 0)
    UIElement e;
#define UI_CHECK_UNCHECKED     (0)
#define UI_CHECK_CHECKED       (1)
#define UI_CHECK_INDETERMINATE (2)
    uint8_t   check;
    char     *label;
    ptrdiff_t labelBytes;
    void (*invoke)(void *cp);
} UICheckbox;


//


UICheckbox *UICheckboxCreate(UIElement *parent, uint32_t flags, const char *label,
                             ptrdiff_t labelBytes);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_CHECKBOX_H_
