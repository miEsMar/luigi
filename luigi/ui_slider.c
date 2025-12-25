#include "ui_slider.h"
#include "ui_draw.h"
#include "ui_window.h"


int _UISliderMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UISlider *slider   = (UISlider *)element;
    bool      vertical = element->flags & UI_SLIDER_VERTICAL;

    if (message == UI_MSG_GET_HEIGHT) {
        return vertical ? UI_SIZE_SLIDER_WIDTH * element->window->scale
                        : UI_SIZE_SLIDER_HEIGHT * element->window->scale;
    } else if (message == UI_MSG_GET_WIDTH) {
        return vertical ? UI_SIZE_SLIDER_HEIGHT * element->window->scale
                        : UI_SIZE_SLIDER_WIDTH * element->window->scale;
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_SLIDER | (vertical ? UI_DRAW_CONTROL_STATE_VERTICAL : 0) |
                          UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      NULL, 0, slider->position, element->window->scale);
    } else if (message == UI_MSG_LEFT_DOWN ||
               (message == UI_MSG_MOUSE_DRAG && element->window->pressedButton == 1)) {
        UIRectangle bounds    = element->bounds;
        int         thumbSize = UI_SIZE_SLIDER_THUMB * element->window->scale;
        double      position  = vertical
                                    ? 1 - ((float)(element->window->cursorY - thumbSize / 2 - bounds.t) /
                                     (UI_RECT_HEIGHT(bounds) - thumbSize))
                                    : (double)(element->window->cursorX - thumbSize / 2 - bounds.l) /
                                    (UI_RECT_WIDTH(bounds) - thumbSize);
        UISliderSetPosition(slider, position, true);
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    }

    return 0;
}


void UISliderSetPosition(UISlider *slider, double position, bool sendChangedMessage)
{
    if (position == slider->position)
        return;
    if (slider->steps > 1)
        position = (int)(position * (slider->steps - 1) + 0.5f) / (float)(slider->steps - 1);
    if (position < 0)
        position = 0;
    if (position > 1)
        position = 1;
    slider->position = position;
    if (sendChangedMessage)
        UIElementMessage(&slider->e, UI_MSG_VALUE_CHANGED, 0, 0);
    UIElementRepaint(&slider->e, NULL);
}

UISlider *UISliderCreate(UIElement *parent, uint32_t flags)
{
    return (UISlider *)UIElementCreate(sizeof(UISlider), parent, flags, _UISliderMessage, "Slider");
}
