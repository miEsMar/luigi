#ifndef LUIGI_UI_H_
#define LUIGI_UI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "font.h"
#include "ui_code.h"
#include "ui_element.h"
#include "ui_table.h"
#include "ui_theme.h"
#include "ui_window.h"


#define UI_CURSOR_ARROW             (0)
#define UI_CURSOR_TEXT              (1)
#define UI_CURSOR_SPLIT_V           (2)
#define UI_CURSOR_SPLIT_H           (3)
#define UI_CURSOR_FLIPPED_ARROW     (4)
#define UI_CURSOR_CROSS_HAIR        (5)
#define UI_CURSOR_HAND              (6)
#define UI_CURSOR_RESIZE_UP         (7)
#define UI_CURSOR_RESIZE_LEFT       (8)
#define UI_CURSOR_RESIZE_UP_RIGHT   (9)
#define UI_CURSOR_RESIZE_UP_LEFT    (10)
#define UI_CURSOR_RESIZE_DOWN       (11)
#define UI_CURSOR_RESIZE_RIGHT      (12)
#define UI_CURSOR_RESIZE_DOWN_RIGHT (13)
#define UI_CURSOR_RESIZE_DOWN_LEFT  (14)
#define UI_CURSOR_COUNT             (15)

#define UI_ALIGN_LEFT   (1)
#define UI_ALIGN_RIGHT  (2)
#define UI_ALIGN_CENTER (3)


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

    // #ifdef UI_DEBUG
    UIWindow *inspector;
    UITable  *inspectorTable;
    UIWindow *inspectorTarget;
    UICode   *inspectorLog;
    // #endif

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


#ifdef UI_INTERNALS
void _UIUpdate(void);
bool _UIDestroy(UIElement *element);
#endif


//


#ifdef __cplusplus
}
#endif


#endif // LUIGI_UI_H_
