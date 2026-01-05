#ifndef LUIGI_PLATFORM_COCOA_H_
#define LUIGI_PLATFORM_COCOA_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_COCOA
# import <Carbon/Carbon.h>
# import <Cocoa/Cocoa.h>
# import <Foundation/Foundation.h>


typedef struct Luigi_Platform_Cocoa {
    void     *menuData[256]; // HACK This limits the number of menu items to 128.
    uintptr_t menuIndex;
    int       menuX, menuY;
    UIWindow *menuWindow;
} Luigi_Platform_Cocoa;


typedef struct Luigi_PlatformWindow_Cocoa {
    NSWindow *window;
    void     *view;
} Luigi_PlatformWindow_Cocoa;
#endif


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_COCOA_H_
