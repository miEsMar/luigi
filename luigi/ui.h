#ifndef LUIGI_UI_H_
#define LUIGI_UI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "font.h"
#include "ui_cursor.h"
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

#ifdef UI_LINUX
    Display *display;
    Visual  *visual;
    XIM      xim;
    Atom     windowClosedID, primaryID, uriListID, plainTextID;
    Atom     dndEnterID, dndLeaveID, dndTypeListID, dndPositionID, dndStatusID, dndActionCopyID,
        dndDropID, dndSelectionID, dndFinishedID, dndAwareID;
    Atom   clipboardID, xSelectionDataID, textID, targetID, incrID;
    Cursor cursors[UI_CURSOR_COUNT];
    char  *pasteText;
    XEvent copyEvent;
    int    epollFD;
#endif

#ifdef UI_WINDOWS
    HCURSOR cursors[UI_CURSOR_COUNT];
    HANDLE  heap;
    bool    assertionFailure;
#endif

#ifdef UI_ESSENCE
    EsInstance *instance;
#endif

#if defined(UI_ESSENCE) || defined(UI_COCOA)
    void     *menuData[256]; // HACK This limits the number of menu items to 128.
    uintptr_t menuIndex;
#endif

#ifdef UI_COCOA
    int       menuX, menuY;
    UIWindow *menuWindow;
#endif

#ifdef UI_FREETYPE
    FT_Library ft;
#endif
};


//


void _UIInitialiseCommon(void);
void _UIUpdate(void);
bool _UIDestroy(UIElement *element);


//


#ifdef __cplusplus
}
#endif


#endif // LUIGI_UI_H_
