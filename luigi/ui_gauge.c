#include "ui_gauge.h"
#include "ui_draw.h"
#include "ui_painter.h"

/////////////////////////////////////////
// Gauges.
/////////////////////////////////////////


UIGauge *UIGaugeCreate(UIElement *parent, uint32_t flags)
{
    return (UIGauge *)UIElementCreate(sizeof(UIGauge), parent, flags, _UIGaugeMessage, "Gauge");
}


void UIGaugeSetPosition(UIGauge *gauge, float position)
{
    if (position == gauge->position)
        return;
    if (position < 0)
        position = 0;
    if (position > 1)
        position = 1;
    gauge->position = position;
    UIElementRepaint(&gauge->e, NULL);
}


int _UIGaugeMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIGauge *gauge    = (UIGauge *)element;
    bool     vertical = element->flags & UI_GAUGE_VERTICAL;

    if (message == UI_MSG_GET_HEIGHT) {
        return vertical ? UI_SIZE_GAUGE_WIDTH * element->window->scale
                        : UI_SIZE_GAUGE_HEIGHT * element->window->scale;
    } else if (message == UI_MSG_GET_WIDTH) {
        return vertical ? UI_SIZE_GAUGE_HEIGHT * element->window->scale
                        : UI_SIZE_GAUGE_WIDTH * element->window->scale;
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_GAUGE | UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element) |
                          (vertical ? UI_DRAW_CONTROL_STATE_VERTICAL : 0),
                      NULL, 0, gauge->position, element->window->scale);
    }

    return 0;
}
