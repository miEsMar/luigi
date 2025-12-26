#ifndef LUIGI_UI_H_
#define LUIGI_UI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "font.h"
#include "ui_element.h"
#include "ui_theme.h"
#include "ui_window.h"


typedef enum UI_Alignment {
    UI_ALIGN_LEFT = 1,
    UI_ALIGN_RIGHT,
    UI_ALIGN_CENTER,
} UI_Alignment;


//


struct Luigi;
extern struct Luigi ui;


struct Luigi {
    UI_Platform *platform;

    UIWindow *windows;
    UITheme   theme;

    UIElement **animating;
    uint32_t    animatingCount;

    UIElement *parentStack[16];
    int        parentStackCount;

    bool        quit;
    const char *dialogResult;
    UIElement  *dialogOldFocus;
    bool        dialogCanExit;

    UIFont *activeFont;
};


//


void Luigi_Init(void);
int  Luigi_Loop(void);


void _UIUpdate(void);


//


#ifdef __cplusplus
}
#endif


#endif // LUIGI_UI_H_
