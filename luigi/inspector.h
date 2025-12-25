#ifndef LUIGI_INSPECTOR_H_
#define LUIGI_INSPECTOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_window.h"


void UIInspectorLog(const char *cFormat, ...);


void _UIInspectorSetFocusedWindow(UIWindow *window);
void _UIInspectorCreate();
void _UIInspectorRefresh(void);


#ifdef __cplusplus
}
#endif

#endif // LUIGI_INSPECTOR_H_
