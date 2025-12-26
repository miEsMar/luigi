#include "ui_textbox.h"
#include "ui.h"
#include "ui_clipboard.h"
#include "ui_code.h"
#include "ui_cursor.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_menu.h"
#include "ui_string.h"
#include "ui_window.h"
#include "utils.h"


//


static int _UITextboxMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UITextbox *textbox = (UITextbox *)element;

    if (message == UI_MSG_GET_HEIGHT) {
        return UI_SIZE_TEXTBOX_HEIGHT * element->window->scale;
    } else if (message == UI_MSG_GET_WIDTH) {
        return UI_SIZE_TEXTBOX_WIDTH * element->window->scale;
    } else if (message == UI_MSG_PAINT) {
        UIDrawControl((UIPainter *)dp, element->bounds,
                      UI_DRAW_CONTROL_TEXTBOX | UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element), NULL,
                      0, 0, element->window->scale);

        int scaledMargin = UI_SIZE_TEXTBOX_MARGIN * element->window->scale;
        int totalWidth   = UIMeasureStringWidth(textbox->string, textbox->bytes) + scaledMargin * 2;
        UIRectangle textBounds = UIRectangleAdd(element->bounds, UI_RECT_1I(scaledMargin));

        if (textbox->scroll > totalWidth - UI_RECT_WIDTH(textBounds)) {
            textbox->scroll = totalWidth - UI_RECT_WIDTH(textBounds);
        }

        if (textbox->scroll < 0) {
            textbox->scroll = 0;
        }

        int caretX = UIMeasureStringWidth(textbox->string, textbox->carets[0]) - textbox->scroll;

        if (caretX < 0) {
            textbox->scroll = caretX + textbox->scroll;
        } else if (caretX > UI_RECT_WIDTH(textBounds)) {
            textbox->scroll = caretX - UI_RECT_WIDTH(textBounds) + textbox->scroll + 1;
        }

#ifdef __cplusplus
        UIStringSelection selection = {};
#else
        UIStringSelection selection = {0};
#endif
        selection.carets[0] =
            _UITextboxByteToColumn(textbox->string, textbox->carets[0], textbox->bytes);
        selection.carets[1] =
            _UITextboxByteToColumn(textbox->string, textbox->carets[1], textbox->bytes);
        selection.colorBackground = ui.theme.selected;
        selection.colorText       = ui.theme.textSelected;
        textBounds.l -= textbox->scroll;

        UIDrawString((UIPainter *)dp, textBounds, textbox->string, textbox->bytes,
                     (element->flags & UI_ELEMENT_DISABLED) ? ui.theme.textDisabled : ui.theme.text,
                     UI_ALIGN_LEFT, element->window->focused == element ? &selection : NULL);
    } else if (message == UI_MSG_GET_CURSOR) {
        return UI_CURSOR_TEXT;
    } else if (message == UI_MSG_LEFT_DOWN) {
        int column = (element->window->cursorX - element->bounds.l + textbox->scroll -
                      UI_SIZE_TEXTBOX_MARGIN * element->window->scale +
                      ((float)ui.activeFont->glyphWidth / 2)) /
                     ui.activeFont->glyphWidth;
        textbox->carets[0] = textbox->carets[1] =
            column <= 0 ? 0 : _UITextboxColumnToByte(textbox->string, column, textbox->bytes);
        UIElementFocus(element);
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(textbox->string);
    } else if (message == UI_MSG_KEY_TYPED) {
        UIKeyTyped *m       = (UIKeyTyped *)dp;
        bool        handled = true;

        if (textbox->rejectNextKey) {
            textbox->rejectNextKey = false;
            handled                = false;
        } else if (m->code == UI_KEYCODE_BACKSPACE || m->code == UI_KEYCODE_DELETE) {
            if (textbox->carets[0] == textbox->carets[1]) {
                UITextboxMoveCaret(textbox, m->code == UI_KEYCODE_BACKSPACE, element->window->ctrl);
            }

            UITextboxReplace(textbox, NULL, 0, true);
        } else if (m->code == UI_KEYCODE_LEFT || m->code == UI_KEYCODE_RIGHT) {
            if (textbox->carets[0] == textbox->carets[1] || element->window->shift) {
                UITextboxMoveCaret(textbox, m->code == UI_KEYCODE_LEFT, element->window->ctrl);
                if (!element->window->shift)
                    textbox->carets[1] = textbox->carets[0];
            } else {
                textbox->carets[1 - element->window->shift] =
                    textbox->carets[element->window->shift];
            }
        } else if (m->code == UI_KEYCODE_HOME || m->code == UI_KEYCODE_END) {
            if (m->code == UI_KEYCODE_HOME) {
                textbox->carets[0] = 0;
            } else {
                textbox->carets[0] = textbox->bytes;
            }

            if (!element->window->shift) {
                textbox->carets[1] = textbox->carets[0];
            }
        } else if (m->code == UI_KEYCODE_LETTER('A') && element->window->ctrl) {
            textbox->carets[1] = 0;
            textbox->carets[0] = textbox->bytes;
        } else if (m->textBytes && !element->window->alt && !element->window->ctrl &&
                   m->text[0] >= 0x20) {
            UITextboxReplace(textbox, m->text, m->textBytes, true);
        } else if ((m->code == UI_KEYCODE_LETTER('C') || m->code == UI_KEYCODE_LETTER('X') ||
                    m->code == UI_KEYCODE_INSERT) &&
                   element->window->ctrl && !element->window->alt && !element->window->shift) {
            _UITextboxCopyText(textbox);

            if (m->code == UI_KEYCODE_LETTER('X')) {
                UITextboxReplace(textbox, NULL, 0, true);
            }
        } else if ((m->code == UI_KEYCODE_LETTER('V') && element->window->ctrl &&
                    !element->window->alt && !element->window->shift) ||
                   (m->code == UI_KEYCODE_INSERT && !element->window->ctrl &&
                    !element->window->alt && element->window->shift)) {
            _UITextboxPasteText(textbox);
        } else {
            handled = false;
        }

        if (handled) {
            UIElementRepaint(element, NULL);
            return 1;
        }
    } else if (message == UI_MSG_RIGHT_DOWN) {
        int c0 = textbox->carets[0], c1 = textbox->carets[1];
        _UITextboxMessage(element, UI_MSG_LEFT_DOWN, di, dp);

        if (c0 < c1 ? (textbox->carets[0] >= c0 && textbox->carets[0] < c1)
                    : (textbox->carets[0] >= c1 && textbox->carets[0] < c0)) {
            textbox->carets[0] = c0,
            textbox->carets[1] = c1; // Only move caret if clicking outside the existing selection.
        }

        UIMenu *menu = UIMenuCreate(&element->window->e, UI_MENU_NO_SCROLL);
        UIMenuAddItem(menu, textbox->carets[0] == textbox->carets[1] ? UI_ELEMENT_DISABLED : 0,
                      "Copy", -1, _UITextboxCopyText, textbox);
        size_t pasteBytes;
        char  *paste = _UIClipboardReadTextStart(textbox->e.window, &pasteBytes);
        UIMenuAddItem(menu, !paste || !pasteBytes ? UI_ELEMENT_DISABLED : 0, "Paste", -1,
                      _UITextboxPasteText, textbox);
        _UIClipboardReadTextEnd(textbox->e.window, paste);
        UIMenuShow(menu);
    }

    return 0;
}


//


int _UITextboxByteToColumn(const char *string, int byte, ptrdiff_t bytes)
{
    return _UIByteToColumn(string, byte, bytes, 4);
}


int _UITextboxColumnToByte(const char *string, int column, ptrdiff_t bytes)
{
    return _UIColumnToByte(string, column, bytes, 4);
}


char *UITextboxToCString(UITextbox *textbox)
{
    char *buffer = (char *)UI_MALLOC(textbox->bytes + 1);

    for (intptr_t i = 0; i < textbox->bytes; i++) {
        buffer[i] = textbox->string[i];
    }

    buffer[textbox->bytes] = 0;
    return buffer;
}


void UITextboxReplace(UITextbox *textbox, const char *text, ptrdiff_t bytes,
                      bool sendChangedMessage)
{
    if (bytes == -1)
        bytes = _UIStringLength(text);
    int deleteFrom = textbox->carets[0], deleteTo = textbox->carets[1];
    if (deleteFrom > deleteTo)
        UI_SWAP(int, deleteFrom, deleteTo);

    UI_MEMMOVE(&textbox->string[deleteFrom], &textbox->string[deleteTo], textbox->bytes - deleteTo);
    textbox->bytes -= deleteTo - deleteFrom;
    textbox->string = (char *)UI_REALLOC(textbox->string, textbox->bytes + bytes);
    UI_MEMMOVE(&textbox->string[deleteFrom + bytes], &textbox->string[deleteFrom],
               textbox->bytes - deleteFrom);
    UI_MEMMOVE(&textbox->string[deleteFrom], &text[0], bytes);
    textbox->bytes += bytes;
    textbox->carets[0] = deleteFrom + bytes;
    textbox->carets[1] = textbox->carets[0];

    if (sendChangedMessage)
        UIElementMessage(&textbox->e, UI_MSG_VALUE_CHANGED, 0, 0);
    textbox->e.window->textboxModifiedFlag = true;
    UIElementRepaint(&textbox->e, NULL);
}


void UITextboxClear(UITextbox *textbox, bool sendChangedMessage)
{
    textbox->carets[1] = 0;
    textbox->carets[0] = textbox->bytes;
    UITextboxReplace(textbox, "", 0, sendChangedMessage);
}


void UITextboxMoveCaret(UITextbox *textbox, bool backward, bool word)
{
    while (true) {
        if (textbox->carets[0] > 0 && backward) {
            _UI_MOVE_CARET_BACKWARD(textbox->carets[0], textbox->string, textbox->carets[0], 0);
        } else if (textbox->carets[0] < textbox->bytes && !backward) {
            _UI_MOVE_CARET_FORWARD(textbox->carets[0], textbox->string, textbox->bytes,
                                   textbox->carets[0]);
        } else {
            return;
        }

        if (!word) {
            return;
        } else if (textbox->carets[0] != textbox->bytes && textbox->carets[0] != 0) {
            _UI_MOVE_CARET_BY_WORD(textbox->string, textbox->bytes, textbox->carets[0]);
        }
    }

    UIElementRepaint(&textbox->e, NULL);
}


void _UITextboxCopyText(void *cp)
{
    UITextbox *textbox = (UITextbox *)cp;

    int to   = textbox->carets[0] > textbox->carets[1] ? textbox->carets[0] : textbox->carets[1];
    int from = textbox->carets[0] < textbox->carets[1] ? textbox->carets[0] : textbox->carets[1];

    if (from != to) {
        char *pasteText = (char *)UI_CALLOC(to - from + 1);
        for (int i = from; i < to; i++)
            pasteText[i - from] = textbox->string[i];
        _UIClipboardWriteText(textbox->e.window, pasteText);
    }
}


void _UITextboxPasteText(void *cp)
{
    UITextbox *textbox = (UITextbox *)cp;
    size_t     bytes;
    char      *text = _UIClipboardReadTextStart(textbox->e.window, &bytes);

    if (text) {
        for (size_t i = 0; i < bytes; i++) {
            if (text[i] == '\n')
                text[i] = ' ';
        }

        UITextboxReplace(textbox, text, bytes, true);
    }

    _UIClipboardReadTextEnd(textbox->e.window, text);
}


UITextbox *UITextboxCreate(UIElement *parent, uint32_t flags)
{
    return (UITextbox *)UIElementCreate(sizeof(UITextbox), parent, flags | UI_ELEMENT_TAB_STOP,
                                        _UITextboxMessage, "Textbox");
}
