#ifndef LUIGI_SLIDER_H_
#define LUIGI_SLIDER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


#define UI_SIZE_SLIDER_WIDTH  (200)
#define UI_SIZE_SLIDER_HEIGHT (25)
#define UI_SIZE_SLIDER_THUMB  (15)
#define UI_SIZE_SLIDER_TRACK  (5)


typedef struct UISlider {
#define UI_SLIDER_VERTICAL (1 << 0)
    UIElement e;
    double    position;
    int       steps;
} UISlider;


//


UISlider *UISliderCreate(UIElement *parent, uint32_t flags);
void      UISliderSetPosition(UISlider *slider, double position, bool sendChangedMessage);


int _UISliderMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_SLIDER_H_
