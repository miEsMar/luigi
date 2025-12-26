#ifndef LUIGI_TEXTBOX_H_
#define LUIGI_TEXTBOX_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


#define UI_SIZE_TEXTBOX_MARGIN (3)
#define UI_SIZE_TEXTBOX_WIDTH  (200)
#define UI_SIZE_TEXTBOX_HEIGHT (27)


typedef struct UITextbox {
#define UI_TEXTBOX_HIDE_CHARACTERS (1 << 0)
    UIElement e;
    char     *string;
    ptrdiff_t bytes;
    int       carets[2];
    int       scroll;
    bool      rejectNextKey;
} UITextbox;


//

UITextbox *UITextboxCreate(UIElement *parent, uint32_t flags);

char *UITextboxToCString(UITextbox *textbox);
void  UITextboxReplace(UITextbox *textbox, const char *text, ptrdiff_t bytes,
                       bool sendChangedMessage);
void  UITextboxMoveCaret(UITextbox *textbox, bool backward, bool word);
void  UITextboxClear(UITextbox *textbox, bool sendChangedMessage);


void _UITextboxCopyText(void *cp);
void _UITextboxPasteText(void *cp);
int  _UITextboxByteToColumn(const char *string, int byte, ptrdiff_t bytes);
int  _UITextboxColumnToByte(const char *string, int column, ptrdiff_t bytes);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_TEXTBOX_H_
