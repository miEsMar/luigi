#include "ui_checkbox.h"
#include "ui_button.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_painter.h"
#include "ui_string.h"
#include "utils.h"


int _UICheckboxMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UICheckbox *box = (UICheckbox *)element;

    if (message == UI_MSG_GET_HEIGHT) {
        return UI_SIZE_BUTTON_HEIGHT * element->window->scale;
    } else if (message == UI_MSG_GET_WIDTH) {
        int labelSize = UIMeasureStringWidth(box->label, box->labelBytes);
        return (labelSize + UI_SIZE_CHECKBOX_BOX + UI_SIZE_CHECKBOX_GAP) * element->window->scale;
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_CHECKBOX |
                          (box->check == UI_CHECK_INDETERMINATE
                               ? UI_DRAW_CONTROL_STATE_INDETERMINATE
                           : box->check == UI_CHECK_CHECKED ? UI_DRAW_CONTROL_STATE_CHECKED
                                                            : 0) |
                          UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
                      box->label, box->labelBytes, 0, element->window->scale);
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(box->label);
    } else if (message == UI_MSG_KEY_TYPED) {
        UIKeyTyped *m = (UIKeyTyped *)dp;

        if (m->textBytes == 1 && m->text[0] == ' ') {
            UIElementMessage(element, UI_MSG_CLICKED, 0, 0);
            UIElementRepaint(element, NULL);
        }
    } else if (message == UI_MSG_CLICKED) {
        box->check =
            (box->check + 1) % ((element->flags & UI_CHECKBOX_ALLOW_INDETERMINATE) ? 3 : 2);
        UIElementRepaint(element, NULL);
        if (box->invoke)
            box->invoke(element->cp);
    }

    return 0;
}

UICheckbox *UICheckboxCreate(UIElement *parent, uint32_t flags, const char *label,
                             ptrdiff_t labelBytes)
{
    UICheckbox *box = (UICheckbox *)UIElementCreate(
        sizeof(UICheckbox), parent, flags | UI_ELEMENT_TAB_STOP, _UICheckboxMessage, "Checkbox");
    box->label = UIStringCopy(label, (box->labelBytes = labelBytes));
    return box;
}
