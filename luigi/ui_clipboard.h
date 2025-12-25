#ifndef LUIGI_CLIPBOARD_H_
#define LUIGI_CLIPBOARD_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_window.h"


void  _UIClipboardWriteText(UIWindow *window, char *text);
char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes);
void  _UIClipboardReadTextEnd(UIWindow *window, char *text);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_CLIPBOARD_H_
