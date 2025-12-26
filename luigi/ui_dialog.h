#ifndef LUIGI_DIALOG_H_
#define LUIGI_DIALOG_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"
#include "ui_window.h"


typedef void (*UIDialogUserCallback)(UIElement *);


const char *UIDialogShow(UIWindow *window, uint32_t flags, const char *format, ...);


void _UIDialogButtonInvoke(void *cp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_DIALOG_H_
