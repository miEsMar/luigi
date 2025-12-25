#include "ui_window.h"
#include "font.h"
#include "inspector.h"
#include "ui.h"
#include "ui_animation.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_menu.h"
#include "ui_painter.h"
#include "ui_theme.h"
#include "utils.h"


//


/////////////////////////////////////////
// Platform layers.
/////////////////////////////////////////


#ifdef UI_LINUX
const int UI_KEYCODE_A         = XK_a;
const int UI_KEYCODE_BACKSPACE = XK_BackSpace;
const int UI_KEYCODE_DELETE    = XK_Delete;
const int UI_KEYCODE_DOWN      = XK_Down;
const int UI_KEYCODE_END       = XK_End;
const int UI_KEYCODE_ENTER     = XK_Return;
const int UI_KEYCODE_ESCAPE    = XK_Escape;
const int UI_KEYCODE_F1        = XK_F1;
const int UI_KEYCODE_HOME      = XK_Home;
const int UI_KEYCODE_LEFT      = XK_Left;
const int UI_KEYCODE_RIGHT     = XK_Right;
const int UI_KEYCODE_SPACE     = XK_space;
const int UI_KEYCODE_TAB       = XK_Tab;
const int UI_KEYCODE_UP        = XK_Up;
const int UI_KEYCODE_INSERT    = XK_Insert;
const int UI_KEYCODE_0         = XK_0;
const int UI_KEYCODE_BACKTICK  = XK_grave;
const int UI_KEYCODE_PAGE_DOWN = XK_Page_Down;
const int UI_KEYCODE_PAGE_UP   = XK_Page_Up;


int _UIWindowMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_DEALLOCATE) {
        UIWindow *window = (UIWindow *)element;
        _UIWindowDestroyCommon(window);
        window->image->data = NULL;
        XDestroyImage(window->image);
        XDestroyIC(window->xic);
        XDestroyWindow(ui.display, ((UIWindow *)element)->window);
        UI_FREE(window->uriList);
    }

    return _UIWindowMessageCommon(element, message, di, dp);
}

UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int _width,
                         int _height)
{
    _UIMenusClose();

    UIWindow *window = (UIWindow *)UIElementCreate(
        sizeof(UIWindow), NULL, flags | UI_ELEMENT_WINDOW, _UIWindowMessage, "Window");
    _UIWindowAdd(window);
    if (owner)
        window->scale = owner->scale;

    int width  = (flags & UI_WINDOW_MENU) ? 1 : _width ? _width : 800;
    int height = (flags & UI_WINDOW_MENU) ? 1 : _height ? _height : 600;

    XSetWindowAttributes attributes = {};
    attributes.override_redirect    = flags & UI_WINDOW_MENU;

    window->window =
        XCreateWindow(ui.display, DefaultRootWindow(ui.display), 0, 0, width, height, 0, 0,
                      InputOutput, CopyFromParent, CWOverrideRedirect, &attributes);
    if (cTitle)
        XStoreName(ui.display, window->window, cTitle);
    XSelectInput(ui.display, window->window,
                 SubstructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask |
                     ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
                     EnterWindowMask | LeaveWindowMask | ButtonMotionMask | KeymapStateMask |
                     FocusChangeMask | PropertyChangeMask);

    if (flags & UI_WINDOW_MAXIMIZE) {
        Atom atoms[2] = {XInternAtom(ui.display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0),
                         XInternAtom(ui.display, "_NET_WM_STATE_MAXIMIZED_VERT", 0)};
        XChangeProperty(ui.display, window->window, XInternAtom(ui.display, "_NET_WM_STATE", 0),
                        XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 2);
    }

    if ((~flags & UI_WINDOW_MENU) && (~flags & UI_ELEMENT_HIDE)) {
        XMapRaised(ui.display, window->window);
    }

    if (flags & UI_WINDOW_CENTER_IN_OWNER) {
        int x = 0, y = 0;
        _UIWindowGetScreenPosition(owner, &x, &y);
        XMoveResizeWindow(ui.display, window->window, x + owner->width / 2 - width / 2,
                          y + owner->height / 2 - height / 2, width, height);
    }

    XSetWMProtocols(ui.display, window->window, &ui.windowClosedID, 1);
    window->image = XCreateImage(ui.display, ui.visual, 24, ZPixmap, 0, NULL, 10, 10, 32, 0);

    window->xic = XCreateIC(ui.xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                            XNClientWindow, window->window, XNFocusWindow, window->window, NULL);

    int dndVersion = 4;
    XChangeProperty(ui.display, window->window, ui.dndAwareID, XA_ATOM, 32 /* bits */,
                    PropModeReplace, (uint8_t *)&dndVersion, 1);

    XFlush(ui.display);

    return window;
}


Display *_UIX11GetDisplay()
{
    // c
    return ui.display;
}


UIWindow *_UIFindWindow(Window window)
{
    UIWindow *w = ui.windows;

    // NOTE: linear search. Consider hashing.
    while (w) {
        if (w->window == window) {
            return w;
        }
        w = w->next;
    }
    return NULL;
}

void UIInitialise()
{
    _UIInitialiseCommon();

    XInitThreads();

    ui.display = XOpenDisplay(NULL);
    ui.visual  = XDefaultVisual(ui.display, 0);

    ui.windowClosedID   = XInternAtom(ui.display, "WM_DELETE_WINDOW", 0);
    ui.primaryID        = XInternAtom(ui.display, "PRIMARY", 0);
    ui.dndEnterID       = XInternAtom(ui.display, "XdndEnter", 0);
    ui.dndLeaveID       = XInternAtom(ui.display, "XdndLeave", 0);
    ui.dndTypeListID    = XInternAtom(ui.display, "XdndTypeList", 0);
    ui.dndPositionID    = XInternAtom(ui.display, "XdndPosition", 0);
    ui.dndStatusID      = XInternAtom(ui.display, "XdndStatus", 0);
    ui.dndActionCopyID  = XInternAtom(ui.display, "XdndActionCopy", 0);
    ui.dndDropID        = XInternAtom(ui.display, "XdndDrop", 0);
    ui.dndSelectionID   = XInternAtom(ui.display, "XdndSelection", 0);
    ui.dndFinishedID    = XInternAtom(ui.display, "XdndFinished", 0);
    ui.dndAwareID       = XInternAtom(ui.display, "XdndAware", 0);
    ui.uriListID        = XInternAtom(ui.display, "text/uri-list", 0);
    ui.plainTextID      = XInternAtom(ui.display, "text/plain", 0);
    ui.clipboardID      = XInternAtom(ui.display, "CLIPBOARD", 0);
    ui.xSelectionDataID = XInternAtom(ui.display, "XSEL_DATA", 0);
    ui.textID           = XInternAtom(ui.display, "TEXT", 0);
    ui.targetID         = XInternAtom(ui.display, "TARGETS", 0);
    ui.incrID           = XInternAtom(ui.display, "INCR", 0);

    ui.cursors[UI_CURSOR_ARROW]             = XCreateFontCursor(ui.display, XC_left_ptr);
    ui.cursors[UI_CURSOR_TEXT]              = XCreateFontCursor(ui.display, XC_xterm);
    ui.cursors[UI_CURSOR_SPLIT_V]           = XCreateFontCursor(ui.display, XC_sb_v_double_arrow);
    ui.cursors[UI_CURSOR_SPLIT_H]           = XCreateFontCursor(ui.display, XC_sb_h_double_arrow);
    ui.cursors[UI_CURSOR_FLIPPED_ARROW]     = XCreateFontCursor(ui.display, XC_right_ptr);
    ui.cursors[UI_CURSOR_CROSS_HAIR]        = XCreateFontCursor(ui.display, XC_crosshair);
    ui.cursors[UI_CURSOR_HAND]              = XCreateFontCursor(ui.display, XC_hand1);
    ui.cursors[UI_CURSOR_RESIZE_UP]         = XCreateFontCursor(ui.display, XC_top_side);
    ui.cursors[UI_CURSOR_RESIZE_LEFT]       = XCreateFontCursor(ui.display, XC_left_side);
    ui.cursors[UI_CURSOR_RESIZE_UP_RIGHT]   = XCreateFontCursor(ui.display, XC_top_right_corner);
    ui.cursors[UI_CURSOR_RESIZE_UP_LEFT]    = XCreateFontCursor(ui.display, XC_top_left_corner);
    ui.cursors[UI_CURSOR_RESIZE_DOWN]       = XCreateFontCursor(ui.display, XC_bottom_side);
    ui.cursors[UI_CURSOR_RESIZE_RIGHT]      = XCreateFontCursor(ui.display, XC_right_side);
    ui.cursors[UI_CURSOR_RESIZE_DOWN_LEFT]  = XCreateFontCursor(ui.display, XC_bottom_left_corner);
    ui.cursors[UI_CURSOR_RESIZE_DOWN_RIGHT] = XCreateFontCursor(ui.display, XC_bottom_right_corner);

    XSetLocaleModifiers("");

    ui.xim = XOpenIM(ui.display, 0, 0, 0);

    if (!ui.xim) {
        XSetLocaleModifiers("@im=none");
        ui.xim = XOpenIM(ui.display, 0, 0, 0);
    }

    ui.epollFD               = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event event = {};
    event.events             = EPOLLIN;
    event.data.ptr           = &ui.display;
    epoll_ctl(ui.epollFD, EPOLL_CTL_ADD, ConnectionNumber(ui.display), &event);
}


void _UIWindowSetCursor(UIWindow *window, int cursor)
{
    XDefineCursor(ui.display, window->window, ui.cursors[cursor]);
}


void _UIX11ResetCursor(UIWindow *window)
{
    XDefineCursor(ui.display, window->window, ui.cursors[UI_CURSOR_ARROW]);
}


void UIMenuShow(UIMenu *menu)
{
    Window child;

    // Find the screen that contains the point the menu was created at.
    Screen *menuScreen = NULL;
    int     screenX, screenY;

    for (int i = 0; i < ScreenCount(ui.display); i++) {
        Screen *screen = ScreenOfDisplay(ui.display, i);
        int     x, y;
        XTranslateCoordinates(ui.display, screen->root, DefaultRootWindow(ui.display), 0, 0, &x, &y,
                              &child);

        if (menu->pointX >= x && menu->pointX < x + screen->width && menu->pointY >= y &&
            menu->pointY < y + screen->height) {
            menuScreen = screen;
            screenX = x, screenY = y;
            break;
        }
    }

    int width, height;
    _UIMenuPrepare(menu, &width, &height);

    {
        // Clamp the menu to the bounds of the window.
        // This step shouldn't be necessary with the screen clamping below, but there are some buggy
        // X11 drivers that report screen sizes incorrectly.
        int       wx, wy;
        UIWindow *parentWindow = menu->parentWindow;
        XTranslateCoordinates(ui.display, parentWindow->window, DefaultRootWindow(ui.display), 0, 0,
                              &wx, &wy, &child);
        if (menu->pointX + width > wx + parentWindow->width)
            menu->pointX = wx + parentWindow->width - width;
        if (menu->pointY + height > wy + parentWindow->height)
            menu->pointY = wy + parentWindow->height - height;
        if (menu->pointX < wx)
            menu->pointX = wx;
        if (menu->pointY < wy)
            menu->pointY = wy;
    }

    if (menuScreen) {
        // Clamp to the bounds of the screen.
        if (menu->pointX + width > screenX + menuScreen->width)
            menu->pointX = screenX + menuScreen->width - width;
        if (menu->pointY + height > screenY + menuScreen->height)
            menu->pointY = screenY + menuScreen->height - height;
        if (menu->pointX < screenX)
            menu->pointX = screenX;
        if (menu->pointY < screenY)
            menu->pointY = screenY;
        if (menu->pointX + width > screenX + menuScreen->width)
            width = screenX + menuScreen->width - menu->pointX;
        if (menu->pointY + height > screenY + menuScreen->height)
            height = screenY + menuScreen->height - menu->pointY;
    }

    Atom properties[] = {
        XInternAtom(ui.display, "_NET_WM_WINDOW_TYPE", true),
        XInternAtom(ui.display, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", true),
        XInternAtom(ui.display, "_MOTIF_WM_HINTS", true),
    };

    XChangeProperty(ui.display, menu->e.window->window, properties[0], XA_ATOM, 32, PropModeReplace,
                    (uint8_t *)properties, 2);
    XSetTransientForHint(ui.display, menu->e.window->window, DefaultRootWindow(ui.display));

    struct Hints {
        int flags;
        int functions;
        int decorations;
        int inputMode;
        int status;
    };

    struct Hints hints = {0};
    hints.flags        = 2;
    XChangeProperty(ui.display, menu->e.window->window, properties[2], properties[2], 32,
                    PropModeReplace, (uint8_t *)&hints, 5);

    XMapWindow(ui.display, menu->e.window->window);
    XMoveResizeWindow(ui.display, menu->e.window->window, menu->pointX, menu->pointY, width,
                      height);
}

void UIWindowPack(UIWindow *window, int _width)
{
    int width  = _width ? _width : UIElementMessage(window->e.children[0], UI_MSG_GET_WIDTH, 0, 0);
    int height = UIElementMessage(window->e.children[0], UI_MSG_GET_HEIGHT, width, 0);
    XResizeWindow(ui.display, window->window, width, height);
}

bool _UIProcessEvent(XEvent *event)
{
    if (event->type == ClientMessage && (Atom)event->xclient.data.l[0] == ui.windowClosedID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        bool exit = !UIElementMessage(&window->e, UI_MSG_WINDOW_CLOSE, 0, 0);
        if (exit)
            return true;
        _UIUpdate();
        return false;
    } else if (event->type == Expose) {
        UIWindow *window = _UIFindWindow(event->xexpose.window);
        if (!window)
            return false;
        XPutImage(ui.display, window->window, DefaultGC(ui.display, 0), window->image, 0, 0, 0, 0,
                  window->width, window->height);
    } else if (event->type == ConfigureNotify) {
        UIWindow *window = _UIFindWindow(event->xconfigure.window);
        if (!window)
            return false;

        if (window->width != event->xconfigure.width ||
            window->height != event->xconfigure.height) {
            window->width  = event->xconfigure.width;
            window->height = event->xconfigure.height;
            window->bits = (uint32_t *)UI_REALLOC(window->bits, window->width * window->height * 4);
            window->image->width          = window->width;
            window->image->height         = window->height;
            window->image->bytes_per_line = window->width * 4;
            window->image->data           = (char *)window->bits;
            window->e.bounds              = UI_RECT_2S(window->width, window->height);
            window->e.clip                = UI_RECT_2S(window->width, window->height);
# ifdef UI_DEBUG
            for (int i = 0; i < window->width * window->height; i++)
                window->bits[i] = 0xFF00FF;
# endif
            UIElementRelayout(&window->e);
            _UIUpdate();
        }
    } else if (event->type == MotionNotify) {
        UIWindow *window = _UIFindWindow(event->xmotion.window);
        if (!window)
            return false;
        window->cursorX = event->xmotion.x;
        window->cursorY = event->xmotion.y;

        if (window->inDrag) {
            // Find a window under the cursor with XdndAware.
            Window dragDestination = DefaultRootWindow(ui.display);
            while (true) {
                if (!dragDestination) {
                    break;
                }

                int32_t propertyCount;
                Atom   *properties = XListProperties(ui.display, dragDestination, &propertyCount);
                bool    aware      = false;

                for (int32_t i = 0; i < propertyCount && !aware; i++) {
                    if (properties[i] == ui.dndAwareID) {
                        aware = true;
                    }
                }

                XFree(properties);
                if (aware) {
                    break;
                }

                int32_t  unused5, unused6, unused0, unused1;
                uint32_t unused2;
                Window   unused3;
                XQueryPointer(ui.display, dragDestination, &unused3, &dragDestination, &unused0,
                              &unused1, &unused5, &unused6, &unused2);
            }

            // Get its XDND version.
            int dragDestinationVersion = -1;
            if (dragDestination == window->dragDestination) {
                dragDestinationVersion = window->dragDestinationVersion;
            } else if (dragDestination != None) {
                window->dragDestinationCanDrop = false; // Window changed.

                Atom          atom;
                int32_t       format;
                unsigned long itemCount, bytesRemaining;
                uint8_t      *data;

                if (Success == XGetWindowProperty(ui.display, dragDestination, ui.dndAwareID, 0, 2,
                                                  False, AnyPropertyType, &atom, &format,
                                                  &itemCount, &bytesRemaining, &data) &&
                    data && format == 32 && itemCount == 1) {
                    dragDestinationVersion = data[0];
                    // printf("dragDestinationVersion = %d\n", dragDestinationVersion);
                }

                XFree(data);
            }

            // Send XdndLeave to the old window.
            if (dragDestination != window->dragDestination &&
                window->dragDestinationVersion != -1) {
                XClientMessageEvent m = {.type         = ClientMessage,
                                         .display      = ui.display,
                                         .window       = window->dragDestination,
                                         .message_type = ui.dndLeaveID,
                                         .format       = 32,
                                         .data         = {.l = {window->window}}};
                XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(ui.display);
                // printf("leave old window\n");
            }

            // Send XdndEnter to the new window.
            if (dragDestination != window->dragDestination && dragDestinationVersion != -1) {
                uint32_t l1 = (dragDestinationVersion < 4 ? dragDestinationVersion : 4) << 24;
                XClientMessageEvent m = {
                    .type         = ClientMessage,
                    .display      = ui.display,
                    .window       = dragDestination,
                    .message_type = ui.dndEnterID,
                    .format       = 32,
                    .data         = {.l = {window->window, l1, ui.uriListID, None, None}}};
                XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(ui.display);
                // printf("enter new window %x\n", l1);
            }

            // Send XdndPosition to the window.
            if (dragDestinationVersion != -1) {
                int32_t  x, y, unused0, unused1;
                uint32_t unused2;
                Window   unused3, unused4;
                XQueryPointer(ui.display, DefaultRootWindow(ui.display), &unused3, &unused4,
                              &unused0, &unused1, &x, &y, &unused2);
                XClientMessageEvent m = {.type         = ClientMessage,
                                         .display      = ui.display,
                                         .window       = dragDestination,
                                         .message_type = ui.dndPositionID,
                                         .format       = 32,
                                         .data         = {.l = {window->window, 0, (x << 16) | y,
                                                                CurrentTime, ui.dndActionCopyID}}};
                XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(ui.display);
            }

            window->dragDestination        = dragDestination;
            window->dragDestinationVersion = dragDestinationVersion;
        } else {
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        }
    } else if (event->type == LeaveNotify) {
        UIWindow *window = _UIFindWindow(event->xcrossing.window);
        if (!window)
            return false;

        if (!window->pressed) {
            window->cursorX = -1;
            window->cursorY = -1;
        }

        _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
    } else if (event->type == ButtonPress || event->type == ButtonRelease) {
        UIWindow *window = _UIFindWindow(event->xbutton.window);
        if (!window)
            return false;
        window->cursorX = event->xbutton.x;
        window->cursorY = event->xbutton.y;

        if (window->inDrag && event->type == ButtonRelease) {
            // Send XdndLeave or XdndDrop.
            if (window->dragDestinationVersion != -1) {
                XClientMessageEvent m = {.type    = ClientMessage,
                                         .display = ui.display,
                                         .window  = window->dragDestination,
                                         .format  = 32,
                                         .data    = {.l = {window->window}}};

                if (window->dragDestinationCanDrop) {
                    m.message_type = ui.dndDropID;
                    m.data.l[2]    = CurrentTime;
                } else {
                    m.message_type = ui.dndLeaveID;
                }

                XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(ui.display);
                // printf("dropped\n");
            }

            window->inDrag = false;
        }

        if (event->xbutton.button >= 1 && event->xbutton.button <= 3) {
            _UIWindowInputEvent(
                window,
                (UIMessage)((event->type == ButtonPress ? UI_MSG_LEFT_DOWN : UI_MSG_LEFT_UP) +
                            event->xbutton.button * 2 - 2),
                0, 0);
        } else if (event->xbutton.button == 4) {
            _UIWindowInputEvent(window, UI_MSG_MOUSE_WHEEL, -72, 0);
        } else if (event->xbutton.button == 5) {
            _UIWindowInputEvent(window, UI_MSG_MOUSE_WHEEL, 72, 0);
        }

        _UIInspectorSetFocusedWindow(window);
    } else if (event->type == KeyPress) {
        UIWindow *window = _UIFindWindow(event->xkey.window);
        if (!window)
            return false;

        if (event->xkey.x == 0x7123 && event->xkey.y == 0x7456) {
            // HACK! See UIWindowPostMessage.
            uintptr_t p = ((uintptr_t)(event->xkey.x_root & 0xFFFF) << 0) |
                          ((uintptr_t)(event->xkey.y_root & 0xFFFF) << 16);
# if INTPTR_MAX == INT64_MAX
            p |= (uintptr_t)(event->xkey.time & 0xFFFFFFFF) << 32;
# endif
            UIElementMessage(&window->e, (UIMessage)event->xkey.state, 0, (void *)p);
            _UIUpdate();
        } else {
            char   text[32];
            KeySym symbol = NoSymbol;
            Status status;
            // printf("%ld, %s\n", symbol, text);
            UIKeyTyped m = {0};
            m.textBytes  = Xutf8LookupString(window->xic, &event->xkey, text, sizeof(text) - 1,
                                             &symbol, &status);
            m.text       = text;
            m.code       = XLookupKeysym(&event->xkey, 0);

            if (symbol == XK_Control_L || symbol == XK_Control_R) {
                window->ctrl     = true;
                window->ctrlCode = event->xkey.keycode;
                _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
            } else if (symbol == XK_Shift_L || symbol == XK_Shift_R) {
                window->shift     = true;
                window->shiftCode = event->xkey.keycode;
                _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
            } else if (symbol == XK_Alt_L || symbol == XK_Alt_R) {
                window->alt     = true;
                window->altCode = event->xkey.keycode;
                _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
            } else if (symbol == XK_KP_Left) {
                m.code = UI_KEYCODE_LEFT;
            } else if (symbol == XK_KP_Right) {
                m.code = UI_KEYCODE_RIGHT;
            } else if (symbol == XK_KP_Up) {
                m.code = UI_KEYCODE_UP;
            } else if (symbol == XK_KP_Down) {
                m.code = UI_KEYCODE_DOWN;
            } else if (symbol == XK_KP_Home) {
                m.code = UI_KEYCODE_HOME;
            } else if (symbol == XK_KP_End) {
                m.code = UI_KEYCODE_END;
            } else if (symbol == XK_KP_Enter) {
                m.code = UI_KEYCODE_ENTER;
            } else if (symbol == XK_KP_Delete) {
                m.code = UI_KEYCODE_DELETE;
            } else if (symbol == XK_KP_Page_Up) {
                m.code = UI_KEYCODE_PAGE_UP;
            } else if (symbol == XK_KP_Page_Down) {
                m.code = UI_KEYCODE_PAGE_DOWN;
            }

            _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
        }
    } else if (event->type == KeyRelease) {
        UIWindow *window = _UIFindWindow(event->xkey.window);
        if (!window)
            return false;

        if (event->xkey.keycode == window->ctrlCode) {
            window->ctrl = false;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        } else if (event->xkey.keycode == window->shiftCode) {
            window->shift = false;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        } else if (event->xkey.keycode == window->altCode) {
            window->alt = false;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        } else {
            char       text[32];
            KeySym     symbol = NoSymbol;
            Status     status;
            UIKeyTyped m = {0};
            m.textBytes  = Xutf8LookupString(window->xic, &event->xkey, text, sizeof(text) - 1,
                                             &symbol, &status);
            m.text       = text;
            m.code       = XLookupKeysym(&event->xkey, 0);
            _UIWindowInputEvent(window, UI_MSG_KEY_RELEASED, 0, &m);
        }
    } else if (event->type == FocusIn) {
        UIWindow *window = _UIFindWindow(event->xfocus.window);
        if (!window)
            return false;
        window->ctrl = window->shift = window->alt = false;
        UIElementMessage(&window->e, UI_MSG_WINDOW_ACTIVATE, 0, 0);
    } else if (event->type == FocusOut || event->type == ResizeRequest) {
        _UIMenusClose();
        _UIUpdate();
    } else if (event->type == ClientMessage && event->xclient.message_type == ui.dndEnterID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        window->dragSource = (Window)event->xclient.data.l[0];
    } else if (event->type == ClientMessage && event->xclient.message_type == ui.dndPositionID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        XClientMessageEvent m = {0};
        m.type                = ClientMessage;
        m.display             = event->xclient.display;
        m.window              = (Window)event->xclient.data.l[0];
        m.message_type        = ui.dndStatusID;
        m.format              = 32;
        m.data.l[0]           = window->window;
        m.data.l[1]           = true;
        m.data.l[4]           = ui.dndActionCopyID;
        XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
        XFlush(ui.display);
    } else if (event->type == ClientMessage && event->xclient.message_type == ui.dndDropID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;

        // TODO Dropping text.

        if (!XConvertSelection(ui.display, ui.dndSelectionID, ui.uriListID, ui.primaryID,
                               window->window, event->xclient.data.l[2])) {
            XClientMessageEvent m = {0};
            m.type                = ClientMessage;
            m.display             = ui.display;
            m.window              = window->dragSource;
            m.message_type        = ui.dndFinishedID;
            m.format              = 32;
            m.data.l[0]           = window->window;
            m.data.l[1]           = 0;
            m.data.l[2]           = ui.dndActionCopyID;
            XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
            XFlush(ui.display);
        }
    } else if (event->type == ClientMessage && event->xclient.message_type == ui.dndStatusID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;

        if (window->inDrag && window->dragDestinationVersion != -1 &&
            window->dragDestination == (Window)event->xclient.data.l[0]) {
            window->dragDestinationCanDrop = event->xclient.data.l[1] & 1;
            // printf("window->dragDestinationCanDrop = %d\n", window->dragDestinationCanDrop);
        }
    } else if (event->type == ClientMessage && event->xclient.message_type == ui.dndFinishedID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        // printf("dnd finished %x\n", (int) event->xclient.data.l[1]);
    } else if (event->type == SelectionNotify) {
        UIWindow *window = _UIFindWindow(event->xselection.requestor);
        if (!window)
            return false;
        if (!window->dragSource)
            return false;

        Atom          type   = None;
        int           format = 0;
        unsigned long count = 0, bytesLeft = 0;
        uint8_t      *data = NULL;
        XGetWindowProperty(ui.display, window->window, ui.primaryID, 0, 65536, False,
                           AnyPropertyType, &type, &format, &count, &bytesLeft, &data);

        if (format == 8 /* bits per character */) {
            if (event->xselection.target == ui.uriListID) {
                char *copy      = (char *)UI_MALLOC(count);
                int   fileCount = 0;

                for (int i = 0; i < (int)count; i++) {
                    copy[i] = data[i];

                    if (i && data[i - 1] == '\r' && data[i] == '\n') {
                        fileCount++;
                    }
                }

                char **files = (char **)UI_MALLOC(sizeof(char *) * fileCount);
                fileCount    = 0;

                for (int i = 0; i < (int)count; i++) {
                    char *s = copy + i;
                    while (!(i && data[i - 1] == '\r' && data[i] == '\n' && i < (int)count))
                        i++;
                    copy[i - 1] = 0;

                    for (int j = 0; s[j]; j++) {
                        if (s[j] == '%' && s[j + 1] && s[j + 2]) {
                            char n[3];
                            n[0] = s[j + 1], n[1] = s[j + 2], n[2] = 0;
                            s[j] = strtol(n, NULL, 16);
                            if (!s[j])
                                break;
                            UI_MEMMOVE(s + j + 1, s + j + 3, strlen(s) - j - 2);
                        }
                    }

                    if (s[0] == 'f' && s[1] == 'i' && s[2] == 'l' && s[3] == 'e' && s[4] == ':' &&
                        s[5] == '/' && s[6] == '/') {
                        files[fileCount++] = s + 7;
                    }
                }

                UIElementMessage(&window->e, UI_MSG_WINDOW_DROP_FILES, fileCount, files);

                UI_FREE(files);
                UI_FREE(copy);
            } else if (event->xselection.target == ui.plainTextID) {
                // TODO.
            }
        }

        XFree(data);

        XClientMessageEvent m = {0};
        m.type                = ClientMessage;
        m.display             = ui.display;
        m.window              = window->dragSource;
        m.message_type        = ui.dndFinishedID;
        m.format              = 32;
        m.data.l[0]           = window->window;
        m.data.l[1]           = true;
        m.data.l[2]           = ui.dndActionCopyID;
        XSendEvent(ui.display, m.window, False, NoEventMask, (XEvent *)&m);
        XFlush(ui.display);

        window->dragSource = 0; // Drag complete.
        _UIUpdate();
    } else if (event->type == SelectionRequest) {
        // printf("SelectionRequest\n");
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        if (XGetSelectionOwner(ui.display, event->xselectionrequest.selection) != window->window)
            return false;
        XSelectionRequestEvent requestEvent         = event->xselectionrequest;
        int                    changePropertyResult = 0;

        if (event->xselectionrequest.selection == ui.dndSelectionID) {
            if (requestEvent.target == ui.uriListID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property,
                    requestEvent.target, 8, PropModeReplace, (const unsigned char *)window->uriList,
                    strlen(window->uriList));
            } else if (requestEvent.target == ui.targetID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property, XA_ATOM,
                    32, PropModeReplace, (unsigned char *)&ui.uriListID, 1);
            }
        } else if (event->xselectionrequest.selection == ui.clipboardID) {
            Atom utf8ID = XInternAtom(ui.display, "UTF8_STRING", 1);
            if (utf8ID == None)
                utf8ID = XA_STRING;

            Atom type = requestEvent.target;
            type      = (type == ui.textID) ? XA_STRING : type;

            if (requestEvent.target == XA_STRING || requestEvent.target == ui.textID ||
                requestEvent.target == utf8ID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property, type, 8,
                    PropModeReplace, (const unsigned char *)ui.pasteText, strlen(ui.pasteText));
            } else if (requestEvent.target == ui.targetID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property, XA_ATOM,
                    32, PropModeReplace, (unsigned char *)&utf8ID, 1);
            }
        } else {
            return false;
        }

        if (changePropertyResult == 0 || changePropertyResult == 1) {
            XSelectionEvent sendEvent = {.type       = SelectionNotify,
                                         .serial     = requestEvent.serial,
                                         .send_event = requestEvent.send_event,
                                         .display    = requestEvent.display,
                                         .requestor  = requestEvent.requestor,
                                         .selection  = requestEvent.selection,
                                         .target     = requestEvent.target,
                                         .property   = requestEvent.property,
                                         .time       = requestEvent.time};

            XSendEvent(ui.display, requestEvent.requestor, 0, 0, (XEvent *)&sendEvent);
        }
    }

    return false;
}

typedef struct UIEpollDispatchPtr {
    void (*fp)(struct UIEpollDispatchPtr *ptr);
} UIEpollDispatchPtr;


bool _UIMessageLoopSingle(int *result)
{
    XEvent events[64];

    if (ui.animatingCount) {
        if (XPending(ui.display)) {
            XNextEvent(ui.display, events + 0);
        } else {
            _UIProcessAnimations();
            return true;
        }
    } else if (XPending(ui.display)) {
        XNextEvent(ui.display, events + 0);
    } else {
        struct epoll_event event;
        int                count = epoll_wait(ui.epollFD, &event, 1, -1);

        if (count != 1) {
        } else if (event.data.ptr == &ui.display) {
            XNextEvent(ui.display, events + 0);
        } else {
            UIEpollDispatchPtr *ptr = (UIEpollDispatchPtr *)event.data.ptr;
            ptr->fp(ptr);
            _UIUpdate();
            return true;
        }
    }

    int p = 1;

    int configureIndex = -1, motionIndex = -1, exposeIndex = -1;

    while (p < 64 && XPending(ui.display)) {
        XNextEvent(ui.display, events + p);

# define _UI_MERGE_EVENTS(a, b)                                                                    \
     if (events[p].type == a) {                                                                    \
         if (b != -1)                                                                              \
             events[b].type = 0;                                                                   \
         b = p;                                                                                    \
     }

        _UI_MERGE_EVENTS(ConfigureNotify, configureIndex);
        _UI_MERGE_EVENTS(MotionNotify, motionIndex);
        _UI_MERGE_EVENTS(Expose, exposeIndex);

        p++;
    }

    for (int i = 0; i < p; i++) {
        if (!events[i].type) {
            continue;
        }

        if (_UIProcessEvent(events + i)) {
            return false;
        }
    }

    return true;
}

void UIDragFilesStart(UIWindow *window, const char **paths, size_t count)
{
    if (window->inDrag) {
        return;
    }
    UI_FREE(window->uriList);

    for (uintptr_t pass = 0, size = 0; pass < 2; pass++) {
        if (pass) {
            window->uriList = UI_MALLOC(size + 1);
            size            = 0;
        }

        for (uintptr_t i = 0; i < count; i++) {
            if (pass) {
                window->uriList[size + 0] = 'f';
                window->uriList[size + 1] = 'i';
                window->uriList[size + 2] = 'l';
                window->uriList[size + 3] = 'e';
                window->uriList[size + 4] = ':';
                window->uriList[size + 5] = '/';
                window->uriList[size + 6] = '/';
            }

            size += 7;

            for (uintptr_t j = 0; paths[i][j]; j++) {
                char c = paths[i][j];

                if (c == ' ' || c == '<' || c == '>' || c == '#' || c == '%' || c == '+' ||
                    c == '{' || c == '}' || c == '|' || c == '\\' || c == '^' || c == '~' ||
                    c == '[' || c == ']' || c == '\'' || c == ';' || c == '?' || c == ':' ||
                    c == '@' || c == '=' || c == '&' || c == '$' || c < 0x20) {
                    if (pass) {
                        const char *hexChars      = "0123456789ABCDEF";
                        window->uriList[size + 0] = '%';
                        window->uriList[size + 1] = hexChars[(c & 0xF0) >> 4];
                        window->uriList[size + 2] = hexChars[c & 0x0F];
                    }

                    size += 3;
                } else {
                    if (pass) {
                        window->uriList[size + 0] = c;
                    }

                    size++;
                }
            }

            if (pass) {
                window->uriList[size + 0] = '\r';
                window->uriList[size + 1] = '\n';
            }

            size += 2;
        }

        if (pass) {
            window->uriList[size] = 0;
        }
    }

    XChangeProperty(ui.display, window->window, ui.dndTypeListID, XA_ATOM, 32, PropModeReplace,
                    (uint8_t *)&ui.uriListID, sizeof(Atom));
    XSetSelectionOwner(ui.display, ui.dndSelectionID, window->window, CurrentTime);
    window->inDrag                 = true;
    window->dragDestination        = None;
    window->dragDestinationVersion = -1;
    window->dragDestinationCanDrop = false;
}

void UIEpollAdd(int fd, UIEpollDispatchPtr *ptr)
{
    struct epoll_event event = {};
    event.events             = EPOLLIN;
    event.data.ptr           = ptr;
    bool success             = 0 == epoll_ctl(ui.epollFD, EPOLL_CTL_ADD, fd, &event);
    UI_ASSERT(success);
}


void UIEpollRemove(int fd)
{
    bool success = 0 == epoll_ctl(ui.epollFD, EPOLL_CTL_DEL, fd, NULL);
    UI_ASSERT(success);
}


void UIWindowPostMessage(UIWindow *window, UIMessage message, void *_dp)
{
    // HACK! Xlib doesn't seem to have a nice way to do this,
    // so send a specially crafted key press event instead.
    // TODO Maybe ClientMessage is what this should use?
    uintptr_t dp    = (uintptr_t)_dp;
    XKeyEvent event = {0};
    event.display   = ui.display;
    event.window    = window->window;
    event.root      = DefaultRootWindow(ui.display);
    event.subwindow = None;
# if INTPTR_MAX == INT64_MAX
    event.time = dp >> 32;
# endif
    event.x           = 0x7123;
    event.y           = 0x7456;
    event.x_root      = (dp >> 0) & 0xFFFF;
    event.y_root      = (dp >> 16) & 0xFFFF;
    event.same_screen = True;
    event.keycode     = 1;
    event.state       = message;
    event.type        = KeyPress;
    XSendEvent(ui.display, window->window, True, KeyPressMask, (XEvent *)&event);
    XFlush(ui.display);
}
#endif // UI_LINUX


#ifdef UI_WINDOWS

void *_UIHeapReAlloc(void *pointer, size_t size);
void *_UIMemmove(void *dest, const void *src, size_t n);


const int UI_KEYCODE_A         = 'A';
const int UI_KEYCODE_0         = '0';
const int UI_KEYCODE_BACKSPACE = VK_BACK;
const int UI_KEYCODE_DELETE    = VK_DELETE;
const int UI_KEYCODE_DOWN      = VK_DOWN;
const int UI_KEYCODE_END       = VK_END;
const int UI_KEYCODE_ENTER     = VK_RETURN;
const int UI_KEYCODE_ESCAPE    = VK_ESCAPE;
const int UI_KEYCODE_F1        = VK_F1;
const int UI_KEYCODE_HOME      = VK_HOME;
const int UI_KEYCODE_LEFT      = VK_LEFT;
const int UI_KEYCODE_RIGHT     = VK_RIGHT;
const int UI_KEYCODE_SPACE     = VK_SPACE;
const int UI_KEYCODE_TAB       = VK_TAB;
const int UI_KEYCODE_UP        = VK_UP;
const int UI_KEYCODE_INSERT    = VK_INSERT;
const int UI_KEYCODE_PAGE_UP   = VK_PRIOR;
const int UI_KEYCODE_PAGE_DOWN = VK_NEXT;

int _UIWindowMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_DEALLOCATE) {
        UIWindow *window = (UIWindow *)element;
        _UIWindowDestroyCommon(window);
        SetWindowLongPtr(window->hwnd, GWLP_USERDATA, 0);
        DestroyWindow(window->hwnd);
    }

    return _UIWindowMessageCommon(element, message, di, dp);
}

LRESULT CALLBACK _UIWindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UIWindow *window = (UIWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!window || ui.assertionFailure) {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    if (message == WM_CLOSE) {
        if (UIElementMessage(&window->e, UI_MSG_WINDOW_CLOSE, 0, 0)) {
            _UIUpdate();
            return 0;
        } else {
            PostQuitMessage(0);
        }
    } else if (message == WM_SIZE) {
        RECT client;
        GetClientRect(hwnd, &client);
        window->width    = client.right;
        window->height   = client.bottom;
        window->bits     = (uint32_t *)UI_REALLOC(window->bits, window->width * window->height * 4);
        window->e.bounds = UI_RECT_2S(window->width, window->height);
        window->e.clip   = UI_RECT_2S(window->width, window->height);
        UIElementRelayout(&window->e);
        _UIUpdate();
    } else if (message == WM_MOUSEMOVE) {
        if (!window->trackingLeave) {
            window->trackingLeave = true;
            TRACKMOUSEEVENT leave = {0};
            leave.cbSize          = sizeof(TRACKMOUSEEVENT);
            leave.dwFlags         = TME_LEAVE;
            leave.hwndTrack       = hwnd;
            TrackMouseEvent(&leave);
        }

        POINT cursor;
        GetCursorPos(&cursor);
        ScreenToClient(hwnd, &cursor);
        window->cursorX = cursor.x;
        window->cursorY = cursor.y;
        _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
    } else if (message == WM_MOUSELEAVE) {
        window->trackingLeave = false;

        if (!window->pressed) {
            window->cursorX = -1;
            window->cursorY = -1;
        }

        _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
    } else if (message == WM_LBUTTONDOWN) {
        SetCapture(hwnd);
        _UIWindowInputEvent(window, UI_MSG_LEFT_DOWN, 0, 0);
    } else if (message == WM_LBUTTONUP) {
        if (window->pressedButton == 1)
            ReleaseCapture();
        _UIWindowInputEvent(window, UI_MSG_LEFT_UP, 0, 0);
    } else if (message == WM_MBUTTONDOWN) {
        SetCapture(hwnd);
        _UIWindowInputEvent(window, UI_MSG_MIDDLE_DOWN, 0, 0);
    } else if (message == WM_MBUTTONUP) {
        if (window->pressedButton == 2)
            ReleaseCapture();
        _UIWindowInputEvent(window, UI_MSG_MIDDLE_UP, 0, 0);
    } else if (message == WM_RBUTTONDOWN) {
        SetCapture(hwnd);
        _UIWindowInputEvent(window, UI_MSG_RIGHT_DOWN, 0, 0);
    } else if (message == WM_RBUTTONUP) {
        if (window->pressedButton == 3)
            ReleaseCapture();
        _UIWindowInputEvent(window, UI_MSG_RIGHT_UP, 0, 0);
    } else if (message == WM_MOUSEWHEEL) {
        int delta = (int)wParam >> 16;
        _UIWindowInputEvent(window, UI_MSG_MOUSE_WHEEL, -delta, 0);
    } else if (message == WM_KEYDOWN) {
        window->ctrl  = GetKeyState(VK_CONTROL) & 0x8000;
        window->shift = GetKeyState(VK_SHIFT) & 0x8000;
        window->alt   = GetKeyState(VK_MENU) & 0x8000;

        UIKeyTyped m = {0};
        m.code       = wParam;
        _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
    } else if (message == WM_CHAR) {
        UIKeyTyped m = {0};
        char       c = wParam;
        m.text       = &c;
        m.textBytes  = 1;
        _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
    } else if (message == WM_PAINT) {
        PAINTSTRUCT      paint;
        HDC              dc   = BeginPaint(hwnd, &paint);
        BITMAPINFOHEADER info = {0};
        info.biSize           = sizeof(info);
        info.biWidth = window->width, info.biHeight = -window->height;
        info.biPlanes = 1, info.biBitCount = 32;
        StretchDIBits(dc, 0, 0, UI_RECT_SIZE(window->e.bounds), 0, 0,
                      UI_RECT_SIZE(window->e.bounds), window->bits, (BITMAPINFO *)&info,
                      DIB_RGB_COLORS, SRCCOPY);
        EndPaint(hwnd, &paint);
    } else if (message == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT) {
        SetCursor(ui.cursors[window->cursorStyle]);
        return 1;
    } else if (message == WM_SETFOCUS || message == WM_KILLFOCUS) {
        _UIMenusClose();

        if (message == WM_SETFOCUS) {
            _UIInspectorSetFocusedWindow(window);
            UIElementMessage(&window->e, UI_MSG_WINDOW_ACTIVATE, 0, 0);
        }
    } else if (message == WM_MOUSEACTIVATE && (window->e.flags & UI_WINDOW_MENU)) {
        return MA_NOACTIVATE;
    } else if (message == WM_DROPFILES) {
        HDROP  drop  = (HDROP)wParam;
        int    count = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);
        char **files = (char **)UI_MALLOC(sizeof(char *) * count);

        for (int i = 0; i < count; i++) {
            int length       = DragQueryFile(drop, i, NULL, 0);
            files[i]         = (char *)UI_MALLOC(length + 1);
            files[i][length] = 0;
            DragQueryFile(drop, i, files[i], length + 1);
        }

        UIElementMessage(&window->e, UI_MSG_WINDOW_DROP_FILES, count, files);
        for (int i = 0; i < count; i++)
            UI_FREE(files[i]);
        UI_FREE(files);
        DragFinish(drop);
        _UIUpdate();
    } else if (message == WM_APP + 1) {
        UIElementMessage(&window->e, (UIMessage)wParam, 0, (void *)lParam);
        _UIUpdate();
    } else {
        if (message == WM_NCLBUTTONDOWN || message == WM_NCMBUTTONDOWN ||
            message == WM_NCRBUTTONDOWN) {
            if (~window->e.flags & UI_WINDOW_MENU) {
                _UIMenusClose();
                _UIUpdate();
            }
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

void UIInitialise()
{
    ui.heap = GetProcessHeap();

    _UIInitialiseCommon();

    ui.cursors[UI_CURSOR_ARROW]             = LoadCursor(NULL, IDC_ARROW);
    ui.cursors[UI_CURSOR_TEXT]              = LoadCursor(NULL, IDC_IBEAM);
    ui.cursors[UI_CURSOR_SPLIT_V]           = LoadCursor(NULL, IDC_SIZENS);
    ui.cursors[UI_CURSOR_SPLIT_H]           = LoadCursor(NULL, IDC_SIZEWE);
    ui.cursors[UI_CURSOR_FLIPPED_ARROW]     = LoadCursor(NULL, IDC_ARROW);
    ui.cursors[UI_CURSOR_CROSS_HAIR]        = LoadCursor(NULL, IDC_CROSS);
    ui.cursors[UI_CURSOR_HAND]              = LoadCursor(NULL, IDC_HAND);
    ui.cursors[UI_CURSOR_RESIZE_UP]         = LoadCursor(NULL, IDC_SIZENS);
    ui.cursors[UI_CURSOR_RESIZE_LEFT]       = LoadCursor(NULL, IDC_SIZEWE);
    ui.cursors[UI_CURSOR_RESIZE_UP_RIGHT]   = LoadCursor(NULL, IDC_SIZENESW);
    ui.cursors[UI_CURSOR_RESIZE_UP_LEFT]    = LoadCursor(NULL, IDC_SIZENWSE);
    ui.cursors[UI_CURSOR_RESIZE_DOWN]       = LoadCursor(NULL, IDC_SIZENS);
    ui.cursors[UI_CURSOR_RESIZE_RIGHT]      = LoadCursor(NULL, IDC_SIZEWE);
    ui.cursors[UI_CURSOR_RESIZE_DOWN_LEFT]  = LoadCursor(NULL, IDC_SIZENESW);
    ui.cursors[UI_CURSOR_RESIZE_DOWN_RIGHT] = LoadCursor(NULL, IDC_SIZENWSE);

    WNDCLASS windowClass      = {0};
    windowClass.lpfnWndProc   = _UIWindowProcedure;
    windowClass.lpszClassName = "normal";
    RegisterClass(&windowClass);
    windowClass.style |= CS_DROPSHADOW;
    windowClass.lpszClassName = "shadow";
    RegisterClass(&windowClass);
}

bool _UIMessageLoopSingle(int *result)
{
    MSG message = {0};

    if (ui.animating) {
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                *result = message.wParam;
                return false;
            }

            TranslateMessage(&message);
            DispatchMessage(&message);
        } else {
            _UIProcessAnimations();
        }
    } else {
        if (!GetMessage(&message, NULL, 0, 0)) {
            *result = message.wParam;
            return false;
        }

        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return true;
}

void UIMenuShow(UIMenu *menu)
{
    int width, height;
    _UIMenuPrepare(menu, &width, &height);
    MoveWindow(menu->e.window->hwnd, menu->pointX, menu->pointY, width, height, FALSE);
    ShowWindow(menu->e.window->hwnd, SW_SHOWNOACTIVATE);
}

UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int width, int height)
{
    _UIMenusClose();

    UIWindow *window = (UIWindow *)UIElementCreate(
        sizeof(UIWindow), NULL, flags | UI_ELEMENT_WINDOW, _UIWindowMessage, "Window");
    _UIWindowAdd(window);
    if (owner)
        window->scale = owner->scale;

    if (flags & UI_WINDOW_MENU) {
        UI_ASSERT(owner);

        window->hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, "shadow", 0, WS_POPUP, 0, 0,
                                      0, 0, owner->hwnd, NULL, NULL, NULL);
    } else {
        window->hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, "normal", cTitle, WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT, width ? width : CW_USEDEFAULT,
                                      height ? height : CW_USEDEFAULT, owner ? owner->hwnd : NULL,
                                      NULL, NULL, NULL);
    }

    SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

    if (~flags & UI_WINDOW_MENU) {
        ShowWindow(window->hwnd, SW_SHOW);
        PostMessage(window->hwnd, WM_SIZE, 0, 0);
    }

    return window;
}

void _UIWindowEndPaint(UIWindow *window, UIPainter *painter)
{
    HDC              dc   = GetDC(window->hwnd);
    BITMAPINFOHEADER info = {0};
    info.biSize           = sizeof(info);
    info.biWidth = window->width, info.biHeight = window->height;
    info.biPlanes = 1, info.biBitCount = 32;
    StretchDIBits(dc, UI_RECT_TOP_LEFT(window->updateRegion), UI_RECT_SIZE(window->updateRegion),
                  window->updateRegion.l, window->updateRegion.b + 1,
                  UI_RECT_WIDTH(window->updateRegion), -UI_RECT_HEIGHT(window->updateRegion),
                  window->bits, (BITMAPINFO *)&info, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(window->hwnd, dc);
}

void _UIWindowSetCursor(UIWindow *window, int cursor) { SetCursor(ui.cursors[cursor]); }

void _UIWindowGetScreenPosition(UIWindow *window, int *_x, int *_y)
{
    POINT p;
    p.x = 0;
    p.y = 0;
    ClientToScreen(window->hwnd, &p);
    *_x = p.x;
    *_y = p.y;
}

void UIWindowPostMessage(UIWindow *window, UIMessage message, void *_dp)
{
    PostMessage(window->hwnd, WM_APP + 1, (WPARAM)message, (LPARAM)_dp);
}

void *_UIHeapReAlloc(void *pointer, size_t size)
{
    if (pointer) {
        if (size) {
            return HeapReAlloc(ui.heap, 0, pointer, size);
        } else {
            UI_FREE(pointer);
            return NULL;
        }
    } else {
        if (size) {
            return UI_MALLOC(size);
        } else {
            return NULL;
        }
    }
}

void _UIClipboardWriteText(UIWindow *window, char *text)
{
    if (OpenClipboard(window->hwnd)) {
        EmptyClipboard();
        HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, _UIStringLength(text) + 1);
        char   *copy   = (char *)GlobalLock(memory);
        for (uintptr_t i = 0; text[i]; i++)
            copy[i] = text[i];
        GlobalUnlock(copy);
        SetClipboardData(CF_TEXT, memory);
        CloseClipboard();
    }
}

char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    if (!OpenClipboard(window->hwnd)) {
        return NULL;
    }

    HANDLE memory = GetClipboardData(CF_TEXT);

    if (!memory) {
        CloseClipboard();
        return NULL;
    }

    char *buffer = (char *)GlobalLock(memory);

    if (!buffer) {
        CloseClipboard();
        return NULL;
    }

    size_t byteCount = GlobalSize(memory);

    if (byteCount < 1) {
        GlobalUnlock(memory);
        CloseClipboard();
        return NULL;
    }

    char *copy = (char *)UI_MALLOC(byteCount + 1);
    for (uintptr_t i = 0; i < byteCount; i++)
        copy[i] = buffer[i];
    copy[byteCount] = 0; // Just in case.

    GlobalUnlock(memory);
    CloseClipboard();

    if (bytes)
        *bytes = _UIStringLength(copy);
    return copy;
}

void _UIClipboardReadTextEnd(UIWindow *window, char *text) { UI_FREE(text); }

void *_UIMemmove(void *dest, const void *src, size_t n)
{
    if ((uintptr_t)dest < (uintptr_t)src) {
        uint8_t       *dest8 = (uint8_t *)dest;
        const uint8_t *src8  = (const uint8_t *)src;
        for (uintptr_t i = 0; i < n; i++) {
            dest8[i] = src8[i];
        }
        return dest;
    } else {
        uint8_t       *dest8 = (uint8_t *)dest;
        const uint8_t *src8  = (const uint8_t *)src;
        for (uintptr_t i = n; i; i--) {
            dest8[i - 1] = src8[i - 1];
        }
        return dest;
    }
}
#endif // UI_WINDOWS


#ifdef UI_ESSENCE
const int UI_KEYCODE_A         = ES_SCANCODE_A;
const int UI_KEYCODE_0         = ES_SCANCODE_0;
const int UI_KEYCODE_BACKSPACE = ES_SCANCODE_BACKSPACE;
const int UI_KEYCODE_DELETE    = ES_SCANCODE_DELETE;
const int UI_KEYCODE_DOWN      = ES_SCANCODE_DOWN_ARROW;
const int UI_KEYCODE_END       = ES_SCANCODE_END;
const int UI_KEYCODE_ENTER     = ES_SCANCODE_ENTER;
const int UI_KEYCODE_ESCAPE    = ES_SCANCODE_ESCAPE;
const int UI_KEYCODE_F1        = ES_SCANCODE_F1;
const int UI_KEYCODE_HOME      = ES_SCANCODE_HOME;
const int UI_KEYCODE_LEFT      = ES_SCANCODE_LEFT_ARROW;
const int UI_KEYCODE_RIGHT     = ES_SCANCODE_RIGHT_ARROW;
const int UI_KEYCODE_SPACE     = ES_SCANCODE_SPACE;
const int UI_KEYCODE_TAB       = ES_SCANCODE_TAB;
const int UI_KEYCODE_UP        = ES_SCANCODE_UP_ARROW;
const int UI_KEYCODE_INSERT    = ES_SCANCODE_INSERT;
const int UI_KEYCODE_PAGE_UP   = ES_SCANCODE_PAGE_UP;
const int UI_KEYCODE_PAGE_DOWN = ES_SCANCODE_PAGE_DOWN;

int _UIWindowMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_DEALLOCATE) {
        // TODO Non-main windows.
        element->window = NULL;
        EsInstanceCloseReference(ui.instance);
    }

    return _UIWindowMessageCommon(element, message, di, dp);
}

void UIInitialise()
{
    _UIInitialiseCommon();

    while (true) {
        EsMessage *message = EsMessageReceive();

        if (message->type == ES_MSG_INSTANCE_CREATE) {
            ui.instance = EsInstanceCreate(message, NULL, 0);
            break;
        }
    }
}

bool _UIMessageLoopSingle(int *result)
{
    if (ui.animating) {
        // TODO.
    } else {
        _UIMessageProcess(EsMessageReceive());
    }

    return true;
}

UIMenu *UIMenuCreate(UIElement *parent, uint32_t flags)
{
    ui.menuIndex = 0;
    return EsMenuCreate(parent->window->window, ES_MENU_AT_CURSOR);
}

void _UIMenuItemCallback(EsMenu *menu, EsGeneric context)
{
    ((void (*)(void *))ui.menuData[context.u * 2 + 0])(ui.menuData[context.u * 2 + 1]);
}

void UIMenuAddItem(UIMenu *menu, uint32_t flags, const char *label, ptrdiff_t labelBytes,
                   void (*invoke)(void *cp), void *cp)
{
    EsAssert(ui.menuIndex < 128);
    ui.menuData[ui.menuIndex * 2 + 0] = (void *)invoke;
    ui.menuData[ui.menuIndex * 2 + 1] = cp;
    EsMenuAddItem(menu, (flags & UI_BUTTON_CHECKED) ? ES_MENU_ITEM_CHECKED : ES_FLAGS_DEFAULT,
                  label, labelBytes, _UIMenuItemCallback, ui.menuIndex);
    ui.menuIndex++;
}


void UIMenuShow(UIMenu *menu)
{
    // c
    EsMenuShow(menu);
}


int _UIWindowCanvasMessage(EsElement *element, EsMessage *message)
{
    UIWindow *window = (UIWindow *)element->window->userData.p;

    if (!window) {
        return 0;
    } else if (message->type == ES_MSG_PAINT) {
        EsRectangle bounds = ES_RECT_4PD(message->painter->offsetX, message->painter->offsetY,
                                         window->width, window->height);
        EsDrawBitmap(message->painter, bounds, window->bits, window->width * 4, 0xFFFF);
    } else if (message->type == ES_MSG_LAYOUT) {
        EsElementGetSize(element, &window->width, &window->height);
        window->bits     = (uint32_t *)UI_REALLOC(window->bits, window->width * window->height * 4);
        window->e.bounds = UI_RECT_2S(window->width, window->height);
        window->e.clip   = UI_RECT_2S(window->width, window->height);
        UIElementRelayout(&window->e);
        _UIUpdate();
    } else if (message->type == ES_MSG_SCROLL_WHEEL) {
        _UIWindowInputEvent(window, UI_MSG_MOUSE_WHEEL, -message->scrollWheel.dy, 0);
    } else if (message->type == ES_MSG_MOUSE_MOVED || message->type == ES_MSG_HOVERED_END ||
               message->type == ES_MSG_MOUSE_LEFT_DRAG ||
               message->type == ES_MSG_MOUSE_RIGHT_DRAG ||
               message->type == ES_MSG_MOUSE_MIDDLE_DRAG) {
        EsPoint point   = EsMouseGetPosition(element);
        window->cursorX = point.x, window->cursorY = point.y;
        _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
    } else if (message->type == ES_MSG_KEY_UP) {
        window->ctrl  = EsKeyboardIsCtrlHeld();
        window->shift = EsKeyboardIsShiftHeld();
        window->alt   = EsKeyboardIsAltHeld();
    } else if (message->type == ES_MSG_KEY_DOWN) {
        window->ctrl  = EsKeyboardIsCtrlHeld();
        window->shift = EsKeyboardIsShiftHeld();
        window->alt   = EsKeyboardIsAltHeld();
        UIKeyTyped m  = {0};
        char       c[64];
        m.text      = c;
        m.textBytes = EsMessageGetInputText(message, c);
        m.code      = message->keyboard.scancode;
        return _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m) ? ES_HANDLED : 0;
    } else if (message->type == ES_MSG_MOUSE_LEFT_CLICK) {
        _UIInspectorSetFocusedWindow(window);
    } else if (message->type == ES_MSG_USER_START) {
        UIElementMessage(&window->e, (UIMessage)message->user.context1.u, 0,
                         (void *)message->user.context2.p);
        _UIUpdate();
    } else if (message->type == ES_MSG_GET_CURSOR) {
        message->cursorStyle = ES_CURSOR_NORMAL;
        if (window->cursor == UI_CURSOR_TEXT)
            message->cursorStyle = ES_CURSOR_TEXT;
        if (window->cursor == UI_CURSOR_SPLIT_V)
            message->cursorStyle = ES_CURSOR_SPLIT_VERTICAL;
        if (window->cursor == UI_CURSOR_SPLIT_H)
            message->cursorStyle = ES_CURSOR_SPLIT_HORIZONTAL;
        if (window->cursor == UI_CURSOR_FLIPPED_ARROW)
            message->cursorStyle = ES_CURSOR_SELECT_LINES;
        if (window->cursor == UI_CURSOR_CROSS_HAIR)
            message->cursorStyle = ES_CURSOR_CROSS_HAIR_PICK;
        if (window->cursor == UI_CURSOR_HAND)
            message->cursorStyle = ES_CURSOR_HAND_HOVER;
        if (window->cursor == UI_CURSOR_RESIZE_UP)
            message->cursorStyle = ES_CURSOR_RESIZE_VERTICAL;
        if (window->cursor == UI_CURSOR_RESIZE_LEFT)
            message->cursorStyle = ES_CURSOR_RESIZE_HORIZONTAL;
        if (window->cursor == UI_CURSOR_RESIZE_UP_RIGHT)
            message->cursorStyle = ES_CURSOR_RESIZE_DIAGONAL_1;
        if (window->cursor == UI_CURSOR_RESIZE_UP_LEFT)
            message->cursorStyle = ES_CURSOR_RESIZE_DIAGONAL_2;
        if (window->cursor == UI_CURSOR_RESIZE_DOWN)
            message->cursorStyle = ES_CURSOR_RESIZE_VERTICAL;
        if (window->cursor == UI_CURSOR_RESIZE_RIGHT)
            message->cursorStyle = ES_CURSOR_RESIZE_HORIZONTAL;
        if (window->cursor == UI_CURSOR_RESIZE_DOWN_RIGHT)
            message->cursorStyle = ES_CURSOR_RESIZE_DIAGONAL_1;
        if (window->cursor == UI_CURSOR_RESIZE_DOWN_LEFT)
            message->cursorStyle = ES_CURSOR_RESIZE_DIAGONAL_2;
    }

    else if (message->type == ES_MSG_MOUSE_LEFT_DOWN)
        _UIWindowInputEvent(window, UI_MSG_LEFT_DOWN, 0, 0);
    else if (message->type == ES_MSG_MOUSE_LEFT_UP)
        _UIWindowInputEvent(window, UI_MSG_LEFT_UP, 0, 0);
    else if (message->type == ES_MSG_MOUSE_MIDDLE_DOWN)
        _UIWindowInputEvent(window, UI_MSG_MIDDLE_DOWN, 0, 0);
    else if (message->type == ES_MSG_MOUSE_MIDDLE_UP)
        _UIWindowInputEvent(window, UI_MSG_MIDDLE_UP, 0, 0);
    else if (message->type == ES_MSG_MOUSE_RIGHT_DOWN)
        _UIWindowInputEvent(window, UI_MSG_RIGHT_DOWN, 0, 0);
    else if (message->type == ES_MSG_MOUSE_RIGHT_UP)
        _UIWindowInputEvent(window, UI_MSG_RIGHT_UP, 0, 0);

    else
        return 0;

    return ES_HANDLED;
}

UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int width, int height)
{
    _UIMenusClose();

    UIWindow *window = (UIWindow *)UIElementCreate(
        sizeof(UIWindow), NULL, flags | UI_ELEMENT_WINDOW, _UIWindowMessage, "Window");
    _UIWindowAdd(window);
    if (owner)
        window->scale = owner->scale;

    if (flags & UI_WINDOW_MENU) {
        // TODO.
    } else {
        // TODO Non-main windows.
        window->window           = ui.instance->window;
        window->window->userData = window;
        window->canvas = EsCustomElementCreate(window->window, ES_CELL_FILL | ES_ELEMENT_FOCUSABLE);
        window->canvas->messageUser = _UIWindowCanvasMessage;
        EsWindowSetTitle(window->window, cTitle, -1);
        EsElementFocus(window->canvas);
    }

    return window;
}

void _UIWindowEndPaint(UIWindow *window, UIPainter *painter)
{
    EsElementRepaint(window->canvas, &window->updateRegion);
}

void _UIWindowSetCursor(UIWindow *window, int cursor) { window->cursor = cursor; }

void _UIWindowGetScreenPosition(UIWindow *window, int *_x, int *_y)
{
    EsRectangle r = EsElementGetScreenBounds(window->window);
    *_x = r.l, *_y = r.t;
}

void UIWindowPostMessage(UIWindow *window, UIMessage message, void *_dp)
{
    EsMessage m       = {};
    m.type            = ES_MSG_USER_START;
    m.user.context1.u = message;
    m.user.context2.p = _dp;
    EsMessagePost(window->canvas, &m);
}

void _UIClipboardWriteText(UIWindow *window, char *text)
{
    EsClipboardAddText(ES_CLIPBOARD_PRIMARY, text, -1);
    UI_FREE(text);
}

char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    return EsClipboardReadText(ES_CLIPBOARD_PRIMARY, bytes, NULL);
}

void _UIClipboardReadTextEnd(UIWindow *window, char *text) { EsHeapFree(text); }
#endif // UI_ESSENCE


#ifdef UI_COCOA
// TODO Standard keyboard shortcuts (Command+Q, Command+W).

const int UI_KEYCODE_A      = -100; // TODO Keyboard layout support.
const int UI_KEYCODE_F1     = -70;
const int UI_KEYCODE_0      = -50;
const int UI_KEYCODE_INSERT = -30;

const int UI_KEYCODE_BACKSPACE = kVK_Delete;
const int UI_KEYCODE_DELETE    = kVK_ForwardDelete;
const int UI_KEYCODE_DOWN      = kVK_DownArrow;
const int UI_KEYCODE_END       = kVK_End;
const int UI_KEYCODE_ENTER     = kVK_Return;
const int UI_KEYCODE_ESCAPE    = kVK_Escape;
const int UI_KEYCODE_HOME      = kVK_Home;
const int UI_KEYCODE_LEFT      = kVK_LeftArrow;
const int UI_KEYCODE_RIGHT     = kVK_RightArrow;
const int UI_KEYCODE_SPACE     = kVK_Space;
const int UI_KEYCODE_TAB       = kVK_Tab;
const int UI_KEYCODE_UP        = kVK_UpArrow;
const int UI_KEYCODE_BACKTICK  = kVK_ANSI_Grave; // TODO Keyboard layout support.
const int UI_KEYCODE_PAGE_UP   = kVK_PageUp;
const int UI_KEYCODE_PAGE_DOWN = kVK_PageDown;

int (*_cocoaAppMain)(int, char **);
int    _cocoaArgc;
char **_cocoaArgv;

struct _UIPostedMessage {
    UIMessage message;
    void     *dp;
};

char *_UIUTF8StringFromNSString(NSString *string)
{
    NSUInteger size   = [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    char      *buffer = (char *)UI_MALLOC(size + 1);
    buffer[size]      = 0;
    [string getBytes:buffer
             maxLength:size
            usedLength:NULL
              encoding:NSUTF8StringEncoding
               options:0
                 range:NSMakeRange(0, [string length])
        remainingRange:NULL];
    return buffer;
}

int _UICocoaRemapKey(int code)
{
    if (code == kVK_ANSI_A) {
        return UI_KEYCODE_LETTER('A');
    }
    if (code == kVK_ANSI_B) {
        return UI_KEYCODE_LETTER('B');
    }
    if (code == kVK_ANSI_C) {
        return UI_KEYCODE_LETTER('C');
    }
    if (code == kVK_ANSI_D) {
        return UI_KEYCODE_LETTER('D');
    }
    if (code == kVK_ANSI_E) {
        return UI_KEYCODE_LETTER('E');
    }
    if (code == kVK_ANSI_F) {
        return UI_KEYCODE_LETTER('F');
    }
    if (code == kVK_ANSI_G) {
        return UI_KEYCODE_LETTER('G');
    }
    if (code == kVK_ANSI_H) {
        return UI_KEYCODE_LETTER('H');
    }
    if (code == kVK_ANSI_I) {
        return UI_KEYCODE_LETTER('I');
    }
    if (code == kVK_ANSI_J) {
        return UI_KEYCODE_LETTER('J');
    }
    if (code == kVK_ANSI_K) {
        return UI_KEYCODE_LETTER('K');
    }
    if (code == kVK_ANSI_L) {
        return UI_KEYCODE_LETTER('L');
    }
    if (code == kVK_ANSI_M) {
        return UI_KEYCODE_LETTER('M');
    }
    if (code == kVK_ANSI_N) {
        return UI_KEYCODE_LETTER('N');
    }
    if (code == kVK_ANSI_O) {
        return UI_KEYCODE_LETTER('O');
    }
    if (code == kVK_ANSI_P) {
        return UI_KEYCODE_LETTER('P');
    }
    if (code == kVK_ANSI_Q) {
        return UI_KEYCODE_LETTER('Q');
    }
    if (code == kVK_ANSI_R) {
        return UI_KEYCODE_LETTER('R');
    }
    if (code == kVK_ANSI_S) {
        return UI_KEYCODE_LETTER('S');
    }
    if (code == kVK_ANSI_T) {
        return UI_KEYCODE_LETTER('T');
    }
    if (code == kVK_ANSI_U) {
        return UI_KEYCODE_LETTER('U');
    }
    if (code == kVK_ANSI_V) {
        return UI_KEYCODE_LETTER('V');
    }
    if (code == kVK_ANSI_W) {
        return UI_KEYCODE_LETTER('W');
    }
    if (code == kVK_ANSI_X) {
        return UI_KEYCODE_LETTER('X');
    }
    if (code == kVK_ANSI_Y) {
        return UI_KEYCODE_LETTER('Y');
    }
    if (code == kVK_ANSI_Z) {
        return UI_KEYCODE_LETTER('Z');
    }

    if (code == kVK_ANSI_0) {
        return UI_KEYCODE_DIGIT('0');
    }
    if (code == kVK_ANSI_1) {
        return UI_KEYCODE_DIGIT('1');
    }
    if (code == kVK_ANSI_2) {
        return UI_KEYCODE_DIGIT('2');
    }
    if (code == kVK_ANSI_3) {
        return UI_KEYCODE_DIGIT('3');
    }
    if (code == kVK_ANSI_4) {
        return UI_KEYCODE_DIGIT('4');
    }
    if (code == kVK_ANSI_5) {
        return UI_KEYCODE_DIGIT('5');
    }
    if (code == kVK_ANSI_6) {
        return UI_KEYCODE_DIGIT('6');
    }
    if (code == kVK_ANSI_7) {
        return UI_KEYCODE_DIGIT('7');
    }
    if (code == kVK_ANSI_8) {
        return UI_KEYCODE_DIGIT('8');
    }
    if (code == kVK_ANSI_9) {
        return UI_KEYCODE_DIGIT('9');
    }

    if (code == kVK_F1) {
        return UI_KEYCODE_FKEY(1);
    }
    if (code == kVK_F2) {
        return UI_KEYCODE_FKEY(2);
    }
    if (code == kVK_F3) {
        return UI_KEYCODE_FKEY(3);
    }
    if (code == kVK_F4) {
        return UI_KEYCODE_FKEY(4);
    }
    if (code == kVK_F5) {
        return UI_KEYCODE_FKEY(5);
    }
    if (code == kVK_F6) {
        return UI_KEYCODE_FKEY(6);
    }
    if (code == kVK_F7) {
        return UI_KEYCODE_FKEY(7);
    }
    if (code == kVK_F8) {
        return UI_KEYCODE_FKEY(8);
    }
    if (code == kVK_F9) {
        return UI_KEYCODE_FKEY(9);
    }
    if (code == kVK_F10) {
        return UI_KEYCODE_FKEY(10);
    }
    if (code == kVK_F11) {
        return UI_KEYCODE_FKEY(11);
    }
    if (code == kVK_F12) {
        return UI_KEYCODE_FKEY(12);
    }

    return code;
}

@interface UICocoaApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@interface                     UICocoaWindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic) UIWindow *uiWindow;
@end

@interface UICocoaMainView : NSView
- (void)handlePostedMessage:(id)message;
- (void)eventCommon:(NSEvent *)event;
@property(nonatomic) UIWindow *uiWindow;
@end

@implementation UICocoaApplicationDelegate
- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    int code = _cocoaAppMain(_cocoaArgc, _cocoaArgv);
    if (code)
        exit(code);
}
@end

@implementation UICocoaWindowDelegate
- (void)windowDidBecomeKey:(NSNotification *)notification
{
    UIElementMessage(&_uiWindow->e, UI_MSG_WINDOW_ACTIVATE, 0, 0);
    _UIUpdate();
}

- (void)windowDidResize:(NSNotification *)notification
{
    _uiWindow->width  = ((UICocoaMainView *)_uiWindow->view).frame.size.width;
    _uiWindow->height = ((UICocoaMainView *)_uiWindow->view).frame.size.height;
    _uiWindow->bits =
        (uint32_t *)UI_REALLOC(_uiWindow->bits, _uiWindow->width * _uiWindow->height * 4);
    _uiWindow->e.bounds = UI_RECT_2S(_uiWindow->width, _uiWindow->height);
    _uiWindow->e.clip   = UI_RECT_2S(_uiWindow->width, _uiWindow->height);
    UIElementRelayout(&_uiWindow->e);
    _UIUpdate();
}
@end

@implementation UICocoaMainView
- (void)handlePostedMessage:(id)_message
{
    _UIPostedMessage *message = (_UIPostedMessage *)_message;
    _UIWindowInputEvent(_uiWindow, message->message, 0, message->dp);
    UI_FREE(message);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)onMenuItemSelected:(NSMenuItem *)menuItem
{
    ((void (*)(void *))ui.menuData[menuItem.tag * 2 + 0])(ui.menuData[menuItem.tag * 2 + 1]);
}

- (void)drawRect:(NSRect)dirtyRect
{
    const unsigned char *data = (const unsigned char *)_uiWindow->bits;
    NSDrawBitmap(NSMakeRect(0, 0, _uiWindow->width, _uiWindow->height), _uiWindow->width,
                 _uiWindow->height, 8 /* bits per channel */, 4 /* channels per pixel */,
                 32 /* bits per pixel */, 4 * _uiWindow->width /* bytes per row */, NO /* planar */,
                 YES /* has alpha */, NSDeviceRGBColorSpace /* color space */, &data /* data */);
}

- (void)eventCommon:(NSEvent *)event
{
    NSPoint cursor     = [self convertPoint:[event locationInWindow] fromView:nil];
    _uiWindow->cursorX = cursor.x, _uiWindow->cursorY = _uiWindow->height - cursor.y - 1;
    _uiWindow->ctrl  = event.modifierFlags & NSEventModifierFlagCommand;
    _uiWindow->shift = event.modifierFlags & NSEventModifierFlagShift;
    _uiWindow->alt   = event.modifierFlags & NSEventModifierFlagOption;
}

- (void)keyDown:(NSEvent *)event
{
    [self eventCommon:event];
    char      *text = _UIUTF8StringFromNSString(event.characters);
    UIKeyTyped m    = {
           .code = _UICocoaRemapKey(event.keyCode), .text = text, .textBytes = (int)strlen(text)};
    _UIWindowInputEvent(_uiWindow, UI_MSG_KEY_TYPED, 0, &m);
    UI_FREE(text);
}

- (void)keyUp:(NSEvent *)event
{
    [self eventCommon:event];
    UIKeyTyped m = {.code = _UICocoaRemapKey(event.keyCode)};
    _UIWindowInputEvent(_uiWindow, UI_MSG_KEY_RELEASED, 0, &m);
}

- (void)mouseMoved:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_MOUSE_MOVE, 0, 0);
}

- (void)mouseExited:(NSEvent *)event
{
    [self mouseMoved:event];
}
- (void)flagsChanged:(NSEvent *)event
{
    [self mouseMoved:event];
}
- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}
- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}
- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseDown:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_LEFT_DOWN, 0, 0);
}

- (void)mouseUp:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_LEFT_UP, 0, 0);
}

- (void)rightMouseDown:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_RIGHT_DOWN, 0, 0);
}

- (void)rightMouseUp:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_RIGHT_UP, 0, 0);
}

- (void)otherMouseDown:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_MIDDLE_DOWN, 0, 0);
}

- (void)otherMouseUp:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_MIDDLE_UP, 0, 0);
}

- (void)scrollWheel:(NSEvent *)event
{
    [self eventCommon:event];
    _UIWindowInputEvent(_uiWindow, UI_MSG_MOUSE_WHEEL, -3 * event.deltaY, 0);
    _UIWindowInputEvent(_uiWindow, UI_MSG_MOUSE_MOVE, 0, 0);
}

// TODO Animations.
// TODO Drag and drop.
// TODO Reporting window close.

@end

int _UIWindowMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_DEALLOCATE) {
        UIWindow *window = (UIWindow *)element;
        _UIWindowDestroyCommon(window);
        [window->window close];
    }

    return _UIWindowMessageCommon(element, message, di, dp);
}

UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int _width,
                         int _height)
{
    _UIMenusClose();
    UIWindow *window = (UIWindow *)UIElementCreate(
        sizeof(UIWindow), NULL, flags | UI_ELEMENT_WINDOW, _UIWindowMessage, "Window");
    _UIWindowAdd(window);
    if (owner)
        window->scale = owner->scale;

    NSRect            frame     = NSMakeRect(0, 0, _width ?: 800, _height ?: 600);
    NSWindowStyleMask styleMask = NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable |
                                  NSWindowStyleMaskResizable | NSWindowStyleMaskTitled;
    NSWindow *nsWindow = [[NSWindow alloc] initWithContentRect:frame
                                                     styleMask:styleMask
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
    [nsWindow center];
    [nsWindow setTitle:@(cTitle ?: "untitled")];
    UICocoaWindowDelegate *delegate = [UICocoaWindowDelegate alloc];
    [delegate setUiWindow:window];
    nsWindow.delegate     = delegate;
    UICocoaMainView *view = [UICocoaMainView alloc];
    window->window        = nsWindow;
    window->view          = view;
    window->width         = frame.size.width;
    window->height        = frame.size.height;
    window->bits     = (uint32_t *)UI_REALLOC(window->bits, window->width * window->height * 4);
    window->e.bounds = UI_RECT_2S(window->width, window->height);
    window->e.clip   = UI_RECT_2S(window->width, window->height);
    [view setUiWindow:window];
    [view initWithFrame:frame];
    nsWindow.contentView = view;
    [view addTrackingArea:[[NSTrackingArea alloc]
                              initWithRect:frame
                                   options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow |
                                           NSTrackingInVisibleRect
                                     owner:view
                                  userInfo:nil]];
    [nsWindow setInitialFirstResponder:view];
    [nsWindow makeKeyAndOrderFront:delegate];

    // TODO UI_WINDOW_MAXIMIZE.

    return window;
}

void _UIClipboardWriteText(UIWindow *window, char *text)
{
    // TODO Clipboard support.
}

char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    // TODO Clipboard support.
    return NULL;
}

void _UIClipboardReadTextEnd(UIWindow *window, char *text) { UI_FREE(text); }

void UIInitialise() { _UIInitialiseCommon(); }

void _UIWindowSetCursor(UIWindow *window, int cursor)
{
    if (cursor == UI_CURSOR_TEXT)
        [[NSCursor IBeamCursor] set];
    else if (cursor == UI_CURSOR_SPLIT_V)
        [[NSCursor resizeUpDownCursor] set];
    else if (cursor == UI_CURSOR_SPLIT_H)
        [[NSCursor resizeLeftRightCursor] set];
    else if (cursor == UI_CURSOR_FLIPPED_ARROW)
        [[NSCursor pointingHandCursor] set];
    else if (cursor == UI_CURSOR_CROSS_HAIR)
        [[NSCursor crosshairCursor] set];
    else if (cursor == UI_CURSOR_HAND)
        [[NSCursor pointingHandCursor] set];
    else
        [[NSCursor arrowCursor] set];
}

void _UIWindowEndPaint(UIWindow *window, UIPainter *painter)
{
    for (int y = painter->clip.t; y < painter->clip.b; y++) {
        for (int x = painter->clip.l; x < painter->clip.r; x++) {
            uint32_t *p = &painter->bits[y * painter->width + x];
            *p = 0xFF000000 | (*p & 0xFF00) | ((*p & 0xFF0000) >> 16) | ((*p & 0xFF) << 16);
        }
    }

    [(UICocoaMainView *)window->view setNeedsDisplayInRect:((UICocoaMainView *)window->view).frame];
}

void _UIWindowGetScreenPosition(UIWindow *window, int *x, int *y)
{
    NSPoint point = [window->window convertPointToScreen:NSMakePoint(0, 0)];
    *x = point.x, *y = point.y;
}

UIMenu *UIMenuCreate(UIElement *parent, uint32_t flags)
{
    // TODO Fix the vertical position.

    if (parent->parent) {
        UIRectangle screenBounds = UIElementScreenBounds(parent);
        ui.menuX                 = screenBounds.l;
        ui.menuY                 = screenBounds.b;
    } else {
        _UIWindowGetScreenPosition(parent->window, &ui.menuX, &ui.menuY);
        ui.menuX += parent->window->cursorX;
        ui.menuY += parent->window->cursorY;
    }

    ui.menuIndex  = 0;
    ui.menuWindow = parent->window;

    NSMenu *menu = [[NSMenu alloc] init];
    [menu setAutoenablesItems:NO];
    return menu;
}

void UIMenuAddItem(UIMenu *menu, uint32_t flags, const char *label, ptrdiff_t labelBytes,
                   void (*invoke)(void *cp), void *cp)
{
    if (ui.menuIndex == 128)
        return;
    ui.menuData[ui.menuIndex * 2 + 0] = (void *)invoke;
    ui.menuData[ui.menuIndex * 2 + 1] = cp;
    NSString *title =
        [[NSString alloc] initWithBytes:label
                                 length:(labelBytes == -1 ? strlen(label) : labelBytes)
                               encoding:NSUTF8StringEncoding];
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:title
                                                  action:@selector(onMenuItemSelected:)
                                           keyEquivalent:@""];
    item.tag         = ui.menuIndex++;
    if (flags & UI_BUTTON_CHECKED)
        [item setState:NSControlStateValueOn];
    [item setEnabled:((flags & UI_ELEMENT_DISABLED) ? NO : YES)];
    [item setTarget:(UICocoaMainView *)ui.menuWindow->view];
    [menu addItem:item];
    [title release];
    [item release];
}

void UIMenuShow(UIMenu *menu)
{
    [menu popUpMenuPositioningItem:nil atLocation:NSMakePoint(ui.menuX, ui.menuY) inView:nil];
    [menu release];
}

void UIWindowPack(UIWindow *window, int _width)
{
    int width  = _width ? _width : UIElementMessage(window->e.children[0], UI_MSG_GET_WIDTH, 0, 0);
    int height = UIElementMessage(window->e.children[0], UI_MSG_GET_HEIGHT, width, 0);
    [window->window setContentSize:NSMakeSize(width, height)];
}

bool _UIMessageLoopSingle(int *result)
{
    // TODO Modal dialog support.
    return false;
}

void UIWindowPostMessage(UIWindow *window, UIMessage _message, void *dp)
{
    _UIPostedMessage *message = (_UIPostedMessage *)UI_MALLOC(sizeof(_UIPostedMessage));
    message->message          = _message;
    message->dp               = dp;
    [(UICocoaMainView *)window->view performSelectorOnMainThread:@selector(handlePostedMessage:)
                                                      withObject:(id)message
                                                   waitUntilDone:NO];
}

int UICocoaMain(int argc, char **argv, int (*appMain)(int, char **))
{
    _cocoaArgc = argc, _cocoaArgv = argv, _cocoaAppMain = appMain;
    NSApplication *application = [NSApplication sharedApplication];
    application.delegate       = [[UICocoaApplicationDelegate alloc] init];
    return NSApplicationMain(argc, (const char **)argv);
}
#endif // UI_COCOA


// ---------------------------
// ---------------------------
// ---------------------------


void _UIWindowEndPaint(UIWindow *window, UIPainter *painter)
{
    (void)painter;

    XPutImage(ui.display, window->window, DefaultGC(ui.display, 0), window->image,
              UI_RECT_TOP_LEFT(window->updateRegion), UI_RECT_TOP_LEFT(window->updateRegion),
              UI_RECT_SIZE(window->updateRegion));
}

void _UIWindowGetScreenPosition(UIWindow *window, int *_x, int *_y)
{
    Window child;
    XTranslateCoordinates(ui.display, window->window, DefaultRootWindow(ui.display), 0, 0, _x, _y,
                          &child);
}


/////////////////////////////////////////
// Common platform layer functionality.
/////////////////////////////////////////


void _UIWindowSetPressed(UIWindow *window, UIElement *element, int button)
{
    UIElement *previous   = window->pressed;
    window->pressed       = element;
    window->pressedButton = button;
    if (previous)
        UIElementMessage(previous, UI_MSG_UPDATE, UI_UPDATE_PRESSED, 0);
    if (element)
        UIElementMessage(element, UI_MSG_UPDATE, UI_UPDATE_PRESSED, 0);

    UIElement *ancestor = element;
    UIElement *child    = NULL;

    while (ancestor) {
        UIElementMessage(ancestor, UI_MSG_PRESSED_DESCENDENT, 0, child);
        child    = ancestor;
        ancestor = ancestor->parent;
    }
}


void _UIWindowDestroyCommon(UIWindow *window)
{
    UI_FREE(window->bits);
    UI_FREE(window->shortcuts);
}

void _UIInitialiseCommon(void)
{
    ui.theme = uiThemeClassic;

#ifdef UI_FREETYPE
    FT_Init_FreeType(&ui.ft);
    UIFontActivate(UIFontCreate(_UI_TO_STRING_2(UI_FONT_PATH), 11));
#else
    UIFontActivate(UIFontCreate(0, 0));
#endif
}

void _UIWindowAdd(UIWindow *window)
{
    window->scale    = 1.0f;
    window->e.window = window;
    window->hovered  = &window->e;
    window->next     = ui.windows;
    ui.windows       = window;
}


int _UIWindowMessageCommon(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message == UI_MSG_LAYOUT && element->childCount) {
        UIElementMove(element->children[0], element->bounds, false);
        if (element->window->dialog)
            UIElementMove(element->window->dialog, element->bounds, false);
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_GET_CHILD_STABILITY) {
        return 3; // Both width and height of the child element are ignored.
    }

    return 0;
}


int UIMessageLoop(void)
{
    _UIInspectorCreate();
    _UIUpdate();
#ifdef UI_AUTOMATION_TESTS
    return UIAutomationRunTests();
#else
    int result = 0;
    while (!ui.quit && _UIMessageLoopSingle(&result))
        ui.dialogResult = NULL;
    return result;
#endif
}
