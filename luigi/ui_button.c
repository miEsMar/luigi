#include "ui_button.h"
#include "ui_element.h"
#include "ui_menu.h"

/////////////////////////////////////////
// Checkboxes and buttons.
/////////////////////////////////////////

int _UIButtonMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UIButton *button     = (UIButton *)element;
    bool      isMenuItem = element->flags & UI_BUTTON_MENU_ITEM;
    bool      isDropDown = element->flags & UI_BUTTON_DROP_DOWN;

    if (message == UI_MSG_GET_HEIGHT) {
        if (isMenuItem) {
            return UI_SIZE_MENU_ITEM_HEIGHT * element->window->scale;
        } else {
            return UI_SIZE_BUTTON_HEIGHT * element->window->scale;
        }
    } else if (message == UI_MSG_GET_WIDTH) {
        int labelSize  = UIMeasureStringWidth(button->label, button->labelBytes);
        int paddedSize = labelSize + UI_SIZE_BUTTON_PADDING * element->window->scale;
        if (isDropDown)
            paddedSize += ui.activeFont->glyphWidth * 2;
        int minimumSize = ((element->flags & UI_BUTTON_SMALL) ? 0
                           : isMenuItem                       ? UI_SIZE_MENU_ITEM_MINIMUM_WIDTH
                                                              : UI_SIZE_BUTTON_MINIMUM_WIDTH) *
                          element->window->scale;
        return paddedSize > minimumSize ? paddedSize : minimumSize;
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl(
            (UIPainter *)dp, element->bounds,
            (isMenuItem   ? UI_DRAW_CONTROL_MENU_ITEM
             : isDropDown ? UI_DRAW_CONTROL_DROP_DOWN
                          : UI_DRAW_CONTROL_PUSH_BUTTON) |
                ((element->flags & UI_BUTTON_CHECKED) ? UI_DRAW_CONTROL_STATE_CHECKED : 0) |
                UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
            button->label, button->labelBytes, 0, element->window->scale);
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(button->label);
    } else if (message == UI_MSG_LEFT_DOWN) {
        if (element->flags & UI_BUTTON_CAN_FOCUS) {
            UIElementFocus(element);
        }
    } else if (message == UI_MSG_KEY_TYPED) {
        UIKeyTyped *m = (UIKeyTyped *)dp;

        if ((m->textBytes == 1 && m->text[0] == ' ') || m->code == UI_KEYCODE_ENTER) {
            UIElementMessage(element, UI_MSG_CLICKED, 0, 0);
            UIElementRepaint(element, NULL);
            return 1;
        }
    } else if (message == UI_MSG_CLICKED) {
        if (button->invoke) {
            button->invoke(element->cp);
        }
    }

    return 0;
}

void UIButtonSetLabel(UIButton *button, const char *string, ptrdiff_t stringBytes)
{
    UI_FREE(button->label);
    button->label = UIStringCopy(string, (button->labelBytes = stringBytes));
    UIElementMeasurementsChanged(&button->e, 1);
    UIElementRepaint(&button->e, NULL);
}

UIButton *UIButtonCreate(UIElement *parent, uint32_t flags, const char *label, ptrdiff_t labelBytes)
{
    UIButton *button = (UIButton *)UIElementCreate(
        sizeof(UIButton), parent, flags | UI_ELEMENT_TAB_STOP, _UIButtonMessage, "Button");
    button->label = UIStringCopy(label, (button->labelBytes = labelBytes));
    return button;
}
