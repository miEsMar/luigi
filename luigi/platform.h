#ifndef LUIGI_PLATFORM_H_
#define LUIGI_PLATFORM_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_LINUX
# include <X11/Xatom.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/cursorfont.h>
# include <sys/epoll.h>
#endif
#ifdef UI_COCOA
# import <Carbon/Carbon.h>
# import <Cocoa/Cocoa.h>
# import <Foundation/Foundation.h>
#endif


#include "ui_cursor.h"
#include "ui_painter.h"


typedef struct UI_Platform {
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
} UI_Platform;


typedef struct UI_PlatformWindow {
#if defined(UI_LINUX)
    Window   window;
    XImage  *image;
    XIC      xic;
    unsigned ctrlCode, shiftCode, altCode;
    Window   dragSource, dragDestination;
    int      dragDestinationVersion;
    bool     inDrag, dragDestinationCanDrop;
    char    *uriList;
#endif
#ifdef UI_WINDOWS
    HWND hwnd;
    bool trackingLeave;
#endif
#ifdef UI_ESSENCE
    EsWindow  *window;
    EsElement *canvas;
    int        cursor;
#endif
#ifdef UI_COCOA
    NSWindow *window;
    void     *view;
#endif
} UI_PlatformWindow;


//

typedef struct UIWindow UIWindow;
typedef struct UIMenu   UIMenu;


UI_Platform *UI_PlatformInit(void);


void UI_Platform_render(UIWindow *window, UIPainter *painter);
void UI_Platform_get_screen_pos(UIWindow *window, int *_x, int *_y);


void UIMenuShow(UIMenu *menu);


//


UIWindow *_UIFindWindow(Window window);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_H_
