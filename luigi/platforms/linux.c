#include "../inspector.h"
#include "../ui.h"
#include "../ui_animation.h"
#include "../ui_event.h"
#include "../ui_key.h"
#include "../ui_menu.h"
#include "../ui_window.h"
#include "../utils.h"


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
