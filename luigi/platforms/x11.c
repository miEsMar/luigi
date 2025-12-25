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


//


void UI_Platform_render(UIWindow *window, UIPainter *painter)
{
    // c

    XPutImage(ui.platform->display, window->window.window, DefaultGC(ui.platform->display, 0),
              window->window.image, UI_RECT_TOP_LEFT(window->updateRegion),
              UI_RECT_TOP_LEFT(window->updateRegion), UI_RECT_SIZE(window->updateRegion));
    return;
}


void UI_Platform_get_screen_pos(UIWindow *window, int *_x, int *_y)
{
    Window child;
    (void)XTranslateCoordinates(ui.platform->display, window->window.window,
                                DefaultRootWindow(ui.platform->display), 0, 0, _x, _y, &child);
    return;
}


//


int _UIWindowMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    const UI_Platform *platform = ui.platform;
    if (message == UI_MSG_DEALLOCATE) {
        UIWindow *window = (UIWindow *)element;
        _UIWindowDestroyCommon(window);
        window->window.image->data = NULL;
        XDestroyImage(window->window.image);
        XDestroyIC(window->window.xic);
        XDestroyWindow(platform->display, ((UIWindow *)element)->window.window);
        UI_FREE(window->window.uriList);
    }

    return _UIWindowMessageCommon(element, message, di, dp);
}


//


UIWindow *UIWindowCreate(UIWindow *owner, uint32_t flags, const char *cTitle, int _width,
                         int _height)
{
    UI_Platform *platform = ui.platform;

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

    window->window.window =
        XCreateWindow(platform->display, DefaultRootWindow(platform->display), 0, 0, width, height,
                      0, 0, InputOutput, CopyFromParent, CWOverrideRedirect, &attributes);
    if (cTitle)
        XStoreName(platform->display, window->window.window, cTitle);
    XSelectInput(platform->display, window->window.window,
                 SubstructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask |
                     ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
                     EnterWindowMask | LeaveWindowMask | ButtonMotionMask | KeymapStateMask |
                     FocusChangeMask | PropertyChangeMask);

    if (flags & UI_WINDOW_MAXIMIZE) {
        Atom atoms[2] = {XInternAtom(platform->display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0),
                         XInternAtom(platform->display, "_NET_WM_STATE_MAXIMIZED_VERT", 0)};
        XChangeProperty(platform->display, window->window.window,
                        XInternAtom(platform->display, "_NET_WM_STATE", 0), XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)atoms, 2);
    }

    if ((~flags & UI_WINDOW_MENU) && (~flags & UI_ELEMENT_HIDE)) {
        XMapRaised(platform->display, window->window.window);
    }

    if (flags & UI_WINDOW_CENTER_IN_OWNER) {
        int x = 0, y = 0;
        _UIWindowGetScreenPosition(owner, &x, &y);
        XMoveResizeWindow(platform->display, window->window.window,
                          x + owner->width / 2 - width / 2, y + owner->height / 2 - height / 2,
                          width, height);
    }

    XSetWMProtocols(platform->display, window->window.window, &platform->windowClosedID, 1);
    window->window.image =
        XCreateImage(platform->display, platform->visual, 24, ZPixmap, 0, NULL, 10, 10, 32, 0);

    window->window.xic =
        XCreateIC(platform->xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow,
                  window->window.window, XNFocusWindow, window->window.window, NULL);

    int dndVersion = 4;
    XChangeProperty(platform->display, window->window.window, platform->dndAwareID, XA_ATOM,
                    32 /* bits */, PropModeReplace, (uint8_t *)&dndVersion, 1);

    XFlush(platform->display);

    return window;
}


Display *_UIX11GetDisplay()
{
    // c
    return ui.platform->display;
}


UIWindow *_UIFindWindow(Window window)
{
    UIWindow *w = ui.windows;

    // NOTE: linear search. Consider hashing.
    while (w) {
        if (w->window.window == window) {
            return w;
        }
        w = w->next;
    }
    return NULL;
}


UI_Platform *UI_PlatformInit(void)
{
    UI_Platform *platform = calloc(1, sizeof(*platform));
    if (NULL == platform) {
        return NULL;
    }

    XInitThreads();

    platform->display = XOpenDisplay(NULL);
    platform->visual  = XDefaultVisual(platform->display, 0);

    platform->windowClosedID   = XInternAtom(platform->display, "WM_DELETE_WINDOW", 0);
    platform->primaryID        = XInternAtom(platform->display, "PRIMARY", 0);
    platform->dndEnterID       = XInternAtom(platform->display, "XdndEnter", 0);
    platform->dndLeaveID       = XInternAtom(platform->display, "XdndLeave", 0);
    platform->dndTypeListID    = XInternAtom(platform->display, "XdndTypeList", 0);
    platform->dndPositionID    = XInternAtom(platform->display, "XdndPosition", 0);
    platform->dndStatusID      = XInternAtom(platform->display, "XdndStatus", 0);
    platform->dndActionCopyID  = XInternAtom(platform->display, "XdndActionCopy", 0);
    platform->dndDropID        = XInternAtom(platform->display, "XdndDrop", 0);
    platform->dndSelectionID   = XInternAtom(platform->display, "XdndSelection", 0);
    platform->dndFinishedID    = XInternAtom(platform->display, "XdndFinished", 0);
    platform->dndAwareID       = XInternAtom(platform->display, "XdndAware", 0);
    platform->uriListID        = XInternAtom(platform->display, "text/uri-list", 0);
    platform->plainTextID      = XInternAtom(platform->display, "text/plain", 0);
    platform->clipboardID      = XInternAtom(platform->display, "CLIPBOARD", 0);
    platform->xSelectionDataID = XInternAtom(platform->display, "XSEL_DATA", 0);
    platform->textID           = XInternAtom(platform->display, "TEXT", 0);
    platform->targetID         = XInternAtom(platform->display, "TARGETS", 0);
    platform->incrID           = XInternAtom(platform->display, "INCR", 0);

    // clang-format off
    platform->cursors[UI_CURSOR_ARROW]             = XCreateFontCursor(platform->display, XC_left_ptr);
    platform->cursors[UI_CURSOR_TEXT]              = XCreateFontCursor(platform->display, XC_xterm);
    platform->cursors[UI_CURSOR_SPLIT_V]           = XCreateFontCursor(platform->display, XC_sb_v_double_arrow);
    platform->cursors[UI_CURSOR_SPLIT_H]           = XCreateFontCursor(platform->display, XC_sb_h_double_arrow);
    platform->cursors[UI_CURSOR_FLIPPED_ARROW]     = XCreateFontCursor(platform->display, XC_right_ptr);
    platform->cursors[UI_CURSOR_CROSS_HAIR]        = XCreateFontCursor(platform->display, XC_crosshair);
    platform->cursors[UI_CURSOR_HAND]              = XCreateFontCursor(platform->display, XC_hand1);
    platform->cursors[UI_CURSOR_RESIZE_UP]         = XCreateFontCursor(platform->display, XC_top_side);
    platform->cursors[UI_CURSOR_RESIZE_LEFT]       = XCreateFontCursor(platform->display, XC_left_side);
    platform->cursors[UI_CURSOR_RESIZE_UP_RIGHT]   = XCreateFontCursor(platform->display, XC_top_right_corner);
    platform->cursors[UI_CURSOR_RESIZE_UP_LEFT]    = XCreateFontCursor(platform->display, XC_top_left_corner);
    platform->cursors[UI_CURSOR_RESIZE_DOWN]       = XCreateFontCursor(platform->display, XC_bottom_side);
    platform->cursors[UI_CURSOR_RESIZE_RIGHT]      = XCreateFontCursor(platform->display, XC_right_side);
    platform->cursors[UI_CURSOR_RESIZE_DOWN_LEFT]  = XCreateFontCursor(platform->display, XC_bottom_left_corner);
    platform->cursors[UI_CURSOR_RESIZE_DOWN_RIGHT] = XCreateFontCursor(platform->display, XC_bottom_right_corner);
    // clang-format on

    XSetLocaleModifiers("");

    platform->xim = XOpenIM(platform->display, 0, 0, 0);

    if (!platform->xim) {
        XSetLocaleModifiers("@im=none");
        platform->xim = XOpenIM(platform->display, 0, 0, 0);
    }

    platform->epollFD        = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event event = {};
    event.events             = EPOLLIN;
    event.data.ptr           = &platform->display;
    epoll_ctl(platform->epollFD, EPOLL_CTL_ADD, ConnectionNumber(platform->display), &event);

    return platform;
}


void _UIWindowSetCursor(UIWindow *window, int cursor)
{
    UI_Platform *platform = ui.platform;
    XDefineCursor(platform->display, window->window.window, platform->cursors[cursor]);
}


void _UIX11ResetCursor(UIWindow *window)
{
    UI_Platform *platform = ui.platform;
    XDefineCursor(platform->display, window->window.window, platform->cursors[UI_CURSOR_ARROW]);
}


void UIMenuShow(UIMenu *menu)
{
    UI_Platform *platform = ui.platform;

    Window child;

    // Find the screen that contains the point the menu was created at.
    Screen *menuScreen = NULL;
    int     screenX, screenY;

    for (int i = 0; i < ScreenCount(platform->display); i++) {
        Screen *screen = ScreenOfDisplay(platform->display, i);
        int     x, y;
        XTranslateCoordinates(platform->display, screen->root, DefaultRootWindow(platform->display),
                              0, 0, &x, &y, &child);

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
        XTranslateCoordinates(platform->display, parentWindow->window.window,
                              DefaultRootWindow(platform->display), 0, 0, &wx, &wy, &child);
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
        XInternAtom(platform->display, "_NET_WM_WINDOW_TYPE", true),
        XInternAtom(platform->display, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", true),
        XInternAtom(platform->display, "_MOTIF_WM_HINTS", true),
    };

    XChangeProperty(platform->display, menu->e.window->window.window, properties[0], XA_ATOM, 32,
                    PropModeReplace, (uint8_t *)properties, 2);
    XSetTransientForHint(platform->display, menu->e.window->window.window,
                         DefaultRootWindow(platform->display));

    struct Hints {
        int flags;
        int functions;
        int decorations;
        int inputMode;
        int status;
    };

    struct Hints hints = {0};
    hints.flags        = 2;
    XChangeProperty(platform->display, menu->e.window->window.window, properties[2], properties[2],
                    32, PropModeReplace, (uint8_t *)&hints, 5);

    XMapWindow(platform->display, menu->e.window->window.window);
    XMoveResizeWindow(platform->display, menu->e.window->window.window, menu->pointX, menu->pointY,
                      width, height);
}


void UIWindowPack(UIWindow *window, int _width)
{
    const UI_Platform *platform = ui.platform;

    int width  = _width ? _width : UIElementMessage(window->e.children[0], UI_MSG_GET_WIDTH, 0, 0);
    int height = UIElementMessage(window->e.children[0], UI_MSG_GET_HEIGHT, width, 0);
    XResizeWindow(platform->display, window->window.window, width, height);
}


bool _UIProcessEvent(XEvent *event)
{
    const UI_Platform *platform = ui.platform;

    if (event->type == ClientMessage &&
        (Atom)event->xclient.data.l[0] == platform->windowClosedID) {
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
        XPutImage(platform->display, window->window.window, DefaultGC(platform->display, 0),
                  window->window.image, 0, 0, 0, 0, window->width, window->height);
    } else if (event->type == ConfigureNotify) {
        UIWindow *window = _UIFindWindow(event->xconfigure.window);
        if (!window)
            return false;

        if (window->width != event->xconfigure.width ||
            window->height != event->xconfigure.height) {
            window->width  = event->xconfigure.width;
            window->height = event->xconfigure.height;
            window->bits = (uint32_t *)UI_REALLOC(window->bits, window->width * window->height * 4);
            window->window.image->width          = window->width;
            window->window.image->height         = window->height;
            window->window.image->bytes_per_line = window->width * 4;
            window->window.image->data           = (char *)window->bits;
            window->e.bounds                     = UI_RECT_2S(window->width, window->height);
            window->e.clip                       = UI_RECT_2S(window->width, window->height);
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

        if (window->window.inDrag) {
            // Find a window under the cursor with XdndAware.
            Window dragDestination = DefaultRootWindow(platform->display);
            while (true) {
                if (!dragDestination) {
                    break;
                }

                int32_t propertyCount;
                Atom   *properties =
                    XListProperties(platform->display, dragDestination, &propertyCount);
                bool aware = false;

                for (int32_t i = 0; i < propertyCount && !aware; i++) {
                    if (properties[i] == platform->dndAwareID) {
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
                XQueryPointer(platform->display, dragDestination, &unused3, &dragDestination,
                              &unused0, &unused1, &unused5, &unused6, &unused2);
            }

            // Get its XDND version.
            int dragDestinationVersion = -1;
            if (dragDestination == window->window.dragDestination) {
                dragDestinationVersion = window->window.dragDestinationVersion;
            } else if (dragDestination != None) {
                window->window.dragDestinationCanDrop = false; // Window changed.

                Atom          atom;
                int32_t       format;
                unsigned long itemCount, bytesRemaining;
                uint8_t      *data;

                if (Success == XGetWindowProperty(platform->display, dragDestination,
                                                  platform->dndAwareID, 0, 2, False,
                                                  AnyPropertyType, &atom, &format, &itemCount,
                                                  &bytesRemaining, &data) &&
                    data && format == 32 && itemCount == 1) {
                    dragDestinationVersion = data[0];
                    // printf("dragDestinationVersion = %d\n", dragDestinationVersion);
                }

                XFree(data);
            }

            // Send XdndLeave to the old window.
            if (dragDestination != window->window.dragDestination &&
                window->window.dragDestinationVersion != -1) {
                XClientMessageEvent m = {.type         = ClientMessage,
                                         .display      = platform->display,
                                         .window       = window->window.dragDestination,
                                         .message_type = platform->dndLeaveID,
                                         .format       = 32,
                                         .data         = {.l = {window->window.window}}};
                XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(platform->display);
                // printf("leave old window\n");
            }

            // Send XdndEnter to the new window.
            if (dragDestination != window->window.dragDestination && dragDestinationVersion != -1) {
                uint32_t l1 = (dragDestinationVersion < 4 ? dragDestinationVersion : 4) << 24;
                XClientMessageEvent m = {
                    .type         = ClientMessage,
                    .display      = platform->display,
                    .window       = dragDestination,
                    .message_type = platform->dndEnterID,
                    .format       = 32,
                    .data = {.l = {window->window.window, l1, platform->uriListID, None, None}}};
                XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(platform->display);
                // printf("enter new window %x\n", l1);
            }

            // Send XdndPosition to the window.
            if (dragDestinationVersion != -1) {
                int32_t  x, y, unused0, unused1;
                uint32_t unused2;
                Window   unused3, unused4;
                XQueryPointer(platform->display, DefaultRootWindow(platform->display), &unused3,
                              &unused4, &unused0, &unused1, &x, &y, &unused2);
                XClientMessageEvent m = {.type         = ClientMessage,
                                         .display      = platform->display,
                                         .window       = dragDestination,
                                         .message_type = platform->dndPositionID,
                                         .format       = 32,
                                         .data = {.l = {window->window.window, 0, (x << 16) | y,
                                                        CurrentTime, platform->dndActionCopyID}}};
                XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(platform->display);
            }

            window->window.dragDestination        = dragDestination;
            window->window.dragDestinationVersion = dragDestinationVersion;
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

        if (window->window.inDrag && event->type == ButtonRelease) {
            // Send XdndLeave or XdndDrop.
            if (window->window.dragDestinationVersion != -1) {
                XClientMessageEvent m = {.type    = ClientMessage,
                                         .display = platform->display,
                                         .window  = window->window.dragDestination,
                                         .format  = 32,
                                         .data    = {.l = {window->window.window}}};

                if (window->window.dragDestinationCanDrop) {
                    m.message_type = platform->dndDropID;
                    m.data.l[2]    = CurrentTime;
                } else {
                    m.message_type = platform->dndLeaveID;
                }

                XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
                XFlush(platform->display);
                // printf("dropped\n");
            }

            window->window.inDrag = false;
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
            m.textBytes  = Xutf8LookupString(window->window.xic, &event->xkey, text,
                                             sizeof(text) - 1, &symbol, &status);
            m.text       = text;
            m.code       = XLookupKeysym(&event->xkey, 0);

            if (symbol == XK_Control_L || symbol == XK_Control_R) {
                window->ctrl            = true;
                window->window.ctrlCode = event->xkey.keycode;
                _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
            } else if (symbol == XK_Shift_L || symbol == XK_Shift_R) {
                window->shift            = true;
                window->window.shiftCode = event->xkey.keycode;
                _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
            } else if (symbol == XK_Alt_L || symbol == XK_Alt_R) {
                window->alt            = true;
                window->window.altCode = event->xkey.keycode;
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

        if (event->xkey.keycode == window->window.ctrlCode) {
            window->ctrl = false;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        } else if (event->xkey.keycode == window->window.shiftCode) {
            window->shift = false;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        } else if (event->xkey.keycode == window->window.altCode) {
            window->alt = false;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, 0);
        } else {
            char       text[32];
            KeySym     symbol = NoSymbol;
            Status     status;
            UIKeyTyped m = {0};
            m.textBytes  = Xutf8LookupString(window->window.xic, &event->xkey, text,
                                             sizeof(text) - 1, &symbol, &status);
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
    } else if (event->type == ClientMessage &&
               event->xclient.message_type == platform->dndEnterID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        window->window.dragSource = (Window)event->xclient.data.l[0];
    } else if (event->type == ClientMessage &&
               event->xclient.message_type == platform->dndPositionID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        XClientMessageEvent m = {0};
        m.type                = ClientMessage;
        m.display             = event->xclient.display;
        m.window              = (Window)event->xclient.data.l[0];
        m.message_type        = platform->dndStatusID;
        m.format              = 32;
        m.data.l[0]           = window->window.window;
        m.data.l[1]           = true;
        m.data.l[4]           = platform->dndActionCopyID;
        XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
        XFlush(platform->display);
    } else if (event->type == ClientMessage && event->xclient.message_type == platform->dndDropID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;

        // TODO Dropping text.

        if (!XConvertSelection(platform->display, platform->dndSelectionID, platform->uriListID,
                               platform->primaryID, window->window.window,
                               event->xclient.data.l[2])) {
            XClientMessageEvent m = {0};
            m.type                = ClientMessage;
            m.display             = platform->display;
            m.window              = window->window.dragSource;
            m.message_type        = platform->dndFinishedID;
            m.format              = 32;
            m.data.l[0]           = window->window.window;
            m.data.l[1]           = 0;
            m.data.l[2]           = platform->dndActionCopyID;
            XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
            XFlush(platform->display);
        }
    } else if (event->type == ClientMessage &&
               event->xclient.message_type == platform->dndStatusID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;

        if (window->window.inDrag && window->window.dragDestinationVersion != -1 &&
            window->window.dragDestination == (Window)event->xclient.data.l[0]) {
            window->window.dragDestinationCanDrop = event->xclient.data.l[1] & 1;
            // printf("window->dragDestinationCanDrop = %d\n", window->dragDestinationCanDrop);
        }
    } else if (event->type == ClientMessage &&
               event->xclient.message_type == platform->dndFinishedID) {
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        // printf("dnd finished %x\n", (int) event->xclient.data.l[1]);
    } else if (event->type == SelectionNotify) {
        UIWindow *window = _UIFindWindow(event->xselection.requestor);
        if (!window)
            return false;
        if (!window->window.dragSource)
            return false;

        Atom          type   = None;
        int           format = 0;
        unsigned long count = 0, bytesLeft = 0;
        uint8_t      *data = NULL;
        XGetWindowProperty(platform->display, window->window.window, platform->primaryID, 0, 65536,
                           False, AnyPropertyType, &type, &format, &count, &bytesLeft, &data);

        if (format == 8 /* bits per character */) {
            if (event->xselection.target == platform->uriListID) {
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
            } else if (event->xselection.target == platform->plainTextID) {
                // TODO.
            }
        }

        XFree(data);

        XClientMessageEvent m = {0};
        m.type                = ClientMessage;
        m.display             = platform->display;
        m.window              = window->window.dragSource;
        m.message_type        = platform->dndFinishedID;
        m.format              = 32;
        m.data.l[0]           = window->window.window;
        m.data.l[1]           = true;
        m.data.l[2]           = platform->dndActionCopyID;
        XSendEvent(platform->display, m.window, False, NoEventMask, (XEvent *)&m);
        XFlush(platform->display);

        window->window.dragSource = 0; // Drag complete.
        _UIUpdate();
    } else if (event->type == SelectionRequest) {
        // printf("SelectionRequest\n");
        UIWindow *window = _UIFindWindow(event->xclient.window);
        if (!window)
            return false;
        if (XGetSelectionOwner(platform->display, event->xselectionrequest.selection) !=
            window->window.window)
            return false;
        XSelectionRequestEvent requestEvent         = event->xselectionrequest;
        int                    changePropertyResult = 0;

        if (event->xselectionrequest.selection == platform->dndSelectionID) {
            if (requestEvent.target == platform->uriListID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property,
                    requestEvent.target, 8, PropModeReplace,
                    (const unsigned char *)window->window.uriList, strlen(window->window.uriList));
            } else if (requestEvent.target == platform->targetID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property, XA_ATOM,
                    32, PropModeReplace, (unsigned char *)&platform->uriListID, 1);
            }
        } else if (event->xselectionrequest.selection == platform->clipboardID) {
            Atom utf8ID = XInternAtom(platform->display, "UTF8_STRING", 1);
            if (utf8ID == None)
                utf8ID = XA_STRING;

            Atom type = requestEvent.target;
            type      = (type == platform->textID) ? XA_STRING : type;

            if (requestEvent.target == XA_STRING || requestEvent.target == platform->textID ||
                requestEvent.target == utf8ID) {
                changePropertyResult = XChangeProperty(
                    requestEvent.display, requestEvent.requestor, requestEvent.property, type, 8,
                    PropModeReplace, (const unsigned char *)platform->pasteText,
                    strlen(platform->pasteText));
            } else if (requestEvent.target == platform->targetID) {
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

            XSendEvent(platform->display, requestEvent.requestor, 0, 0, (XEvent *)&sendEvent);
        }
    }

    return false;
}


typedef struct UIEpollDispatchPtr {
    void (*fp)(struct UIEpollDispatchPtr *ptr);
} UIEpollDispatchPtr;


bool _UIMessageLoopSingle(int *result)
{
    const UI_Platform *platform = ui.platform;

    XEvent events[64];

    if (ui.animatingCount) {
        if (XPending(platform->display)) {
            XNextEvent(platform->display, events + 0);
        } else {
            _UIProcessAnimations();
            return true;
        }
    } else if (XPending(platform->display)) {
        XNextEvent(platform->display, events + 0);
    } else {
        struct epoll_event event;
        int                count = epoll_wait(platform->epollFD, &event, 1, -1);

        if (count != 1) {
        } else if (event.data.ptr == &platform->display) {
            XNextEvent(platform->display, events + 0);
        } else {
            UIEpollDispatchPtr *ptr = (UIEpollDispatchPtr *)event.data.ptr;
            ptr->fp(ptr);
            _UIUpdate();
            return true;
        }
    }

    int p = 1;

    int configureIndex = -1, motionIndex = -1, exposeIndex = -1;

    while (p < 64 && XPending(platform->display)) {
        XNextEvent(platform->display, events + p);

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
    const UI_Platform *platform = ui.platform;

    if (window->window.inDrag) {
        return;
    }
    UI_FREE(window->window.uriList);

    for (uintptr_t pass = 0, size = 0; pass < 2; pass++) {
        if (pass) {
            window->window.uriList = UI_MALLOC(size + 1);
            size                   = 0;
        }

        for (uintptr_t i = 0; i < count; i++) {
            if (pass) {
                window->window.uriList[size + 0] = 'f';
                window->window.uriList[size + 1] = 'i';
                window->window.uriList[size + 2] = 'l';
                window->window.uriList[size + 3] = 'e';
                window->window.uriList[size + 4] = ':';
                window->window.uriList[size + 5] = '/';
                window->window.uriList[size + 6] = '/';
            }

            size += 7;

            for (uintptr_t j = 0; paths[i][j]; j++) {
                char c = paths[i][j];

                if (c == ' ' || c == '<' || c == '>' || c == '#' || c == '%' || c == '+' ||
                    c == '{' || c == '}' || c == '|' || c == '\\' || c == '^' || c == '~' ||
                    c == '[' || c == ']' || c == '\'' || c == ';' || c == '?' || c == ':' ||
                    c == '@' || c == '=' || c == '&' || c == '$' || c < 0x20) {
                    if (pass) {
                        const char *hexChars             = "0123456789ABCDEF";
                        window->window.uriList[size + 0] = '%';
                        window->window.uriList[size + 1] = hexChars[(c & 0xF0) >> 4];
                        window->window.uriList[size + 2] = hexChars[c & 0x0F];
                    }

                    size += 3;
                } else {
                    if (pass) {
                        window->window.uriList[size + 0] = c;
                    }

                    size++;
                }
            }

            if (pass) {
                window->window.uriList[size + 0] = '\r';
                window->window.uriList[size + 1] = '\n';
            }

            size += 2;
        }

        if (pass) {
            window->window.uriList[size] = 0;
        }
    }

    XChangeProperty(platform->display, window->window.window, platform->dndTypeListID, XA_ATOM, 32,
                    PropModeReplace, (uint8_t *)&platform->uriListID, sizeof(Atom));
    XSetSelectionOwner(platform->display, platform->dndSelectionID, window->window.window,
                       CurrentTime);
    window->window.inDrag                 = true;
    window->window.dragDestination        = None;
    window->window.dragDestinationVersion = -1;
    window->window.dragDestinationCanDrop = false;
}


void UIEpollAdd(int fd, UIEpollDispatchPtr *ptr)
{
    struct epoll_event event = {};
    event.events             = EPOLLIN;
    event.data.ptr           = ptr;
    bool success             = 0 == epoll_ctl(ui.platform->epollFD, EPOLL_CTL_ADD, fd, &event);
    UI_ASSERT(success);
}


void UIEpollRemove(int fd)
{
    bool success = 0 == epoll_ctl(ui.platform->epollFD, EPOLL_CTL_DEL, fd, NULL);
    UI_ASSERT(success);
}


void UIWindowPostMessage(UIWindow *window, UIMessage message, void *_dp)
{
    const UI_Platform *platform = ui.platform;

    // HACK! Xlib doesn't seem to have a nice way to do this,
    // so send a specially crafted key press event instead.
    // TODO Maybe ClientMessage is what this should use?
    uintptr_t dp    = (uintptr_t)_dp;
    XKeyEvent event = {0};
    event.display   = platform->display;
    event.window    = window->window.window;
    event.root      = DefaultRootWindow(platform->display);
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
    XSendEvent(platform->display, window->window.window, True, KeyPressMask, (XEvent *)&event);
    XFlush(platform->display);
}
#endif // UI_LINUX
