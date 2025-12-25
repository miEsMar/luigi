#ifndef LUIGI_COLOR_H_
#define LUIGI_COLOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


typedef struct UIColorPicker {
#define UI_COLOR_PICKER_HAS_OPACITY (1 << 0)
    UIElement e;
    float     hue, saturation, value, opacity;
} UIColorPicker;


#ifdef __cplusplus
}
#endif

#endif // LUIGI_COLOR_H_
