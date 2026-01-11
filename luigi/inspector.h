#ifndef LUIGI_INSPECTOR_H_
#define LUIGI_INSPECTOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_code.h"
#include "ui_table.h"
#include "ui_window.h"


typedef struct UIInspector {
    UIWindow *window;
    UITable  *inspectorTable;
    UIWindow *inspectorTarget;
    UICode   *inspectorLog;
} UIInspector;


//


void Luigi_InspectorCreate(void);
void UIInspectorLog(const char *cFormat, ...);

void _UIInspectorSetFocusedWindow(UIWindow *window);
void _UIInspectorRefresh(void);


#ifdef __cplusplus
}
#endif

#endif // LUIGI_INSPECTOR_H_
