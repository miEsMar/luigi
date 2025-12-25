#ifndef LUIGI_ANIMATION_H_
#define LUIGI_ANIMATION_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"
#include <stdbool.h>


bool     UIElementAnimate(UIElement *element, bool stop);
uint64_t UIAnimateClock(void);


void _UIProcessAnimations(void);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_ANIMATION_H_
