#include "ui_dialog.h"
#include "ui.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_label.h"
#include "ui_pane.h"
#include "ui_panel.h"
#include "ui_spacer.h"
#include "ui_textbox.h"
#include "utils.h"
#include <stdarg.h>


/////////////////////////////////////////
// Modal dialogs.
/////////////////////////////////////////


static int _UIDialogWrapperMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_LAYOUT) {
        int         width  = UIElementMessage(element->children[0], UI_MSG_GET_WIDTH, 0, 0);
        int         height = UIElementMessage(element->children[0], UI_MSG_GET_HEIGHT, width, 0);
        int         cx     = (element->bounds.l + element->bounds.r) / 2;
        int         cy     = (element->bounds.t + element->bounds.b) / 2;
        UIRectangle bounds =
            UI_RECT_4(cx - (width + 1) / 2, cx + width / 2, cy - (height + 1) / 2, cy + height / 2);
        UIElementMove(element->children[0], bounds, false);
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->children[0]->bounds, UI_DRAW_CONTROL_MODAL_POPUP,
                      NULL, 0, 0, element->window->scale);
    } else if (message == UI_MSG_KEY_TYPED) {
        UIKeyTyped *typed = (UIKeyTyped *)dp;

        if (element->window->ctrl)
            return 0;
        if (element->window->shift)
            return 0;

        if (!ui.dialogCanExit) {
        } else if (!element->window->alt && typed->code == UI_KEYCODE_ESCAPE) {
            ui.dialogResult = "__C";
            return 1;
        } else if (!element->window->alt && typed->code == UI_KEYCODE_ENTER) {
            ui.dialogResult = "__D";
            return 1;
        }

        char c0 = 0, c1 = 0;

        if (typed->textBytes == 1 && typed->text[0] >= 'a' && typed->text[0] <= 'z') {
            c0 = typed->text[0], c1 = typed->text[0] - 'a' + 'A';
        } else {
            return 0;
        }

        UIElement *rowContainer = element->children[0];
        UIElement *target       = NULL;
        bool       duplicate    = false;

        for (uint32_t i = 0; i < rowContainer->childCount; i++) {
            for (uint32_t j = 0; j < rowContainer->children[i]->childCount; j++) {
                UIElement *item = rowContainer->children[i]->children[j];

                if (item->messageClass == _UIButtonMessage) {
                    UIButton *button = (UIButton *)item;

                    if (button->label && button->labelBytes &&
                        (button->label[0] == c0 || button->label[0] == c1)) {
                        if (!target) {
                            target = &button->e;
                        } else {
                            duplicate = true;
                        }
                    }
                }
            }
        }

        if (target) {
            if (duplicate) {
                UIElementFocus(target);
            } else {
                UIElementMessage(target, UI_MSG_CLICKED, 0, 0);
            }

            return 1;
        }
    }

    return 0;
}


static int _UIDialogDefaultButtonMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_PAINT && element->window->focused->messageClass != _UIButtonMessage) {
        element->flags |= UI_BUTTON_CHECKED;
        element->messageClass(element, message, di, dp);
        element->flags &= ~UI_BUTTON_CHECKED;
        return 1;
    }

    return 0;
}


static int _UIDialogTextboxMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UITextbox *textbox = (UITextbox *)element;

    if (message == UI_MSG_VALUE_CHANGED) {
        char **buffer             = (char **)element->cp;
        *buffer                   = (char *)UI_REALLOC(*buffer, textbox->bytes + 1);
        (*buffer)[textbox->bytes] = 0;

        for (ptrdiff_t i = 0; i < textbox->bytes; i++) {
            (*buffer)[i] = textbox->string[i];
        }
    } else if (message == UI_MSG_UPDATE && di == UI_UPDATE_FOCUSED &&
               element->window->focused == element) {
        textbox->carets[1] = 0;
        textbox->carets[0] = textbox->bytes;
        UIElementRepaint(element, NULL);
    }

    return 0;
}


//


const char *UIDialogShow(UIWindow *window, uint32_t flags, const char *format, ...)
{
    // Create the dialog wrapper and panel.

    UI_ASSERT(!window->dialog);
    window->dialog =
        UIElementCreate(sizeof(UIElement), &window->e, 0, _UIDialogWrapperMessage, "DialogWrapper");
    UIPanel *panel = UIPanelCreate(window->dialog, UI_PANEL_MEDIUM_SPACING | UI_PANEL_COLOR_1);
    panel->border  = UI_RECT_1(UI_SIZE_PANE_MEDIUM_BORDER * 2);
    window->e.children[0]->flags |= UI_ELEMENT_DISABLED;

    // Create the dialog contents.

    va_list arguments;
    va_start(arguments, format);
    UIPanel   *row           = NULL;
    UIElement *focus         = NULL;
    UIButton  *defaultButton = NULL;
    UIButton  *cancelButton  = NULL;
    uint32_t   buttonCount   = 0;

    for (int i = 0; format[i]; i++) {
        if (i == 0 || format[i - 1] == '\n') {
            row      = UIPanelCreate(&panel->e, UI_PANEL_HORIZONTAL | UI_ELEMENT_H_FILL);
            row->gap = UI_SIZE_PANE_SMALL_GAP;
        }

        if (format[i] == ' ' || format[i] == '\n') {
        } else if (format[i] == '%') {
            i++;

            if (format[i] == 'b' /* button */ || format[i] == 'B' /* default button */ ||
                format[i] == 'C' /* cancel button */) {
                const char *label  = va_arg(arguments, const char *);
                UIButton   *button = UIButtonCreate(&row->e, 0, label, -1);
                if (!focus)
                    focus = &button->e;
                if (format[i] == 'B')
                    defaultButton = button;
                if (format[i] == 'C')
                    cancelButton = button;
                buttonCount++;
                button->invoke = _UIDialogButtonInvoke;
                if (format[i] == 'B')
                    button->e.messageUser = _UIDialogDefaultButtonMessage;
                button->e.cp = (void *)label;
            } else if (format[i] == 's' /* label from string */) {
                const char *label = va_arg(arguments, const char *);
                UILabelCreate(&row->e, 0, label, -1);
            } else if (format[i] == 't' /* textbox */) {
                char     **buffer  = va_arg(arguments, char **);
                UITextbox *textbox = UITextboxCreate(&row->e, UI_ELEMENT_H_FILL);
                if (!focus)
                    focus = &textbox->e;
                if (*buffer)
                    UITextboxReplace(textbox, *buffer, _UIStringLength(*buffer), false);
                textbox->e.cp          = buffer;
                textbox->e.messageUser = _UIDialogTextboxMessage;
            } else if (format[i] == 'f' /* horizontal fill */) {
                UISpacerCreate(&row->e, UI_ELEMENT_H_FILL, 0, 0);
            } else if (format[i] == 'l' /* horizontal line */) {
                UISpacerCreate(&row->e, UI_ELEMENT_BORDER | UI_ELEMENT_H_FILL, 0, 1);
            } else if (format[i] == 'u' /* user */) {
                UIDialogUserCallback callback = va_arg(arguments, UIDialogUserCallback);
                callback(&row->e);
            }
        } else {
            int j = i;
            while (format[j] && format[j] != '%' && format[j] != '\n')
                j++;
            UILabelCreate(&row->e, 0, format + i, j - i);
            i = j - 1;
        }
    }

    va_end(arguments);

    window->dialogOldFocus = window->focused;
    UIElementFocus(focus ? focus : window->dialog);

    // Run the modal message loop.

    int result;
    ui.dialogResult  = NULL;
    ui.dialogCanExit = buttonCount != 0;
    for (int i = 1; i <= 3; i++)
        _UIWindowSetPressed(window, NULL, i);
    UIElementRefresh(&window->e);
    Luigi_UpdateUI();
    while (!ui.dialogResult && _UIMessageLoopSingle(&result))
        ;
    ui.quit = !ui.dialogResult;

    // Check for cancel/default action.

    if (buttonCount == 1 && defaultButton && !cancelButton) {
        cancelButton = defaultButton;
    }

    if (!ui.dialogResult) {
    } else if (ui.dialogResult[0] == '_' && ui.dialogResult[1] == '_' &&
               ui.dialogResult[2] == 'C' && ui.dialogResult[3] == 0 && cancelButton) {
        ui.dialogResult = (const char *)cancelButton->e.cp;
    } else if (ui.dialogResult[0] == '_' && ui.dialogResult[1] == '_' &&
               ui.dialogResult[2] == 'D' && ui.dialogResult[3] == 0 && defaultButton) {
        ui.dialogResult = (const char *)defaultButton->e.cp;
    }

    // Destroy the dialog.

    window->e.children[0]->flags &= ~UI_ELEMENT_DISABLED;
    UIElementDestroy(window->dialog);
    window->dialog = NULL;
    UIElementRefresh(&window->e);
    if (window->dialogOldFocus)
        UIElementFocus(window->dialogOldFocus);
    return ui.dialogResult ? ui.dialogResult : "";
}


void _UIDialogButtonInvoke(void *cp)
{
    // c
    ui.dialogResult = (const char *)cp;
}
