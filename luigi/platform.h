#ifndef LUIGI_PLATFORM_H_
#define LUIGI_PLATFORM_H_


#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_LINUX
# include <X11/Xatom.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/cursorfont.h>
# include <stdbool.h>
# include <sys/epoll.h>
#endif
#ifdef UI_COCOA
# import <Carbon/Carbon.h>
# import <Cocoa/Cocoa.h>
# import <Foundation/Foundation.h>
#endif
#ifdef UI_WINDOWS
# undef _UNICODE
# undef UNICODE
# include <windows.h>
#endif


typedef struct Luigi_PlatformWindow {
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
} Luigi_PlatformWindow;


#include "ui_painter.h"


//


typedef void *Luigi_Platform;
// typedef void *Luigi_PlatformWindow;

typedef struct UIWindow UIWindow;
typedef struct UIMenu   UIMenu;


Luigi_Platform *Luigi_PlatformInit(void);
UIWindow       *Luigi_Platform_CreateWindow(UIWindow *owner, uint32_t flags, const char *cTitle,
                                            int _width, int _height);
void            Luigi_Platform_get_screen_pos(Luigi_PlatformWindow *pwindow, int *_x, int *_y);
void            Luigi_Platform_render(UIWindow *window, UIPainter *painter);


void UIMenuShow(UIMenu *menu);


//


#ifdef UI_LINUX
UIWindow *_UIFindWindow(Window window);
#endif
#ifdef UI_WINDOWS
void *_UIHeapReAlloc(void *pointer, size_t size);
void *_UIMemmove(void *dest, const void *src, size_t n);
#endif


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_H_
