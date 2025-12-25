#ifndef LUIGI_DIALOG_H_
#define LUIGI_DIALOG_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"
#include "ui_window.h"


typedef void (*UIDialogUserCallback)(UIElement *);


const char *UIDialogShow(UIWindow *window, uint32_t flags, const char *format, ...);


int  _UIDialogWrapperMessage(UIElement *element, UIMessage message, int di, void *dp);
void _UIDialogButtonInvoke(void *cp);
int  _UIDialogDefaultButtonMessage(UIElement *element, UIMessage message, int di, void *dp);
int  _UIDialogTextboxMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_DIALOG_H_
