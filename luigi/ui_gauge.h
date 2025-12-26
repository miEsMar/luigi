#ifndef LUIGI_GAUGE_H_
#define LUIGI_GAUGE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


#define UI_SIZE_GAUGE_WIDTH  (200)
#define UI_SIZE_GAUGE_HEIGHT (22)


#define UI_GAUGE_VERTICAL (1 << 0)


typedef struct UIGauge {
    UIElement e;
    double    position;
} UIGauge;


//


UIGauge *UIGaugeCreate(UIElement *parent, uint32_t flags);
void     UIGaugeSetPosition(UIGauge *gauge, float position);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_GAUGE_H_
