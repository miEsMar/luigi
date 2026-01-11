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


void Luigi_Inspector_Create(void);
void Luigi_Inspector_Update(void);
void Luigi_Inspector_SetFocusedWindow(UIWindow *window);


#ifdef __cplusplus
}
#endif

#endif // LUIGI_INSPECTOR_H_
