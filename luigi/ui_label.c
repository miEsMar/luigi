#include "ui_label.h"
#include "ui_draw.h"
#include "ui_window.h"
#include "utils.h"


int _UILabelMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UILabel *label = (UILabel *)element;

    if (message == UI_MSG_GET_HEIGHT) {
        return UIMeasureStringHeight();
    } else if (message == UI_MSG_GET_WIDTH) {
        return UIMeasureStringWidth(label->label, label->labelBytes);
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_LABEL | UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      label->label, label->labelBytes, 0, element->window->scale);
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(label->label);
    }

    return 0;
}

void UILabelSetContent(UILabel *label, const char *string, ptrdiff_t stringBytes)
{
    UI_FREE(label->label);
    label->label = UIStringCopy(string, (label->labelBytes = stringBytes));
    UIElementMeasurementsChanged(&label->e, 1);
    UIElementRepaint(&label->e, NULL);
}

UILabel *UILabelCreate(UIElement *parent, uint32_t flags, const char *string, ptrdiff_t stringBytes)
{
    UILabel *label =
        (UILabel *)UIElementCreate(sizeof(UILabel), parent, flags, _UILabelMessage, "Label");
    label->label = UIStringCopy(string, (label->labelBytes = stringBytes));
    return label;
}
