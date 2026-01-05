#ifndef LUIGI_PLATFORM_WINDOWS_H_
#define LUIGI_PLATFORM_WINDOWS_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_WINDOWS
# undef _UNICODE
# undef UNICODE
# include <windows.h>


typedef struct Luigi_Platform_Windows {
    HCURSOR cursors[UI_CURSOR_COUNT];
    bool    assertionFailure;
} Luigi_Platform_Windows;


typedef struct Luigi_PlatformWindow_Windows {
    HWND hwnd;
    bool trackingLeave;
} Luigi_PlatformWindow_Windows;
#endif

#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_WINDOWS_H_
