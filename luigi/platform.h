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


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_H_
