#ifndef LUIGI_PLATFORM_X11_H_
#define LUIGI_PLATFORM_X11_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_LINUX
# include "../ui_cursor.h"
# include <X11/Xatom.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/cursorfont.h>
# include <stdbool.h>
# include <sys/epoll.h>


typedef struct Luigi_Platform_X11 {
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
} Luigi_Platform_X11;


typedef struct Luigi_PlatformWindow_X11 {
    Window   window;
    XImage  *image;
    XIC      xic;
    unsigned ctrlCode, shiftCode, altCode;
    Window   dragSource, dragDestination;
    int      dragDestinationVersion;
    bool     inDrag, dragDestinationCanDrop;
    char    *uriList;
} Luigi_PlatformWindow_X11;
#endif

#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_X11_H_
