#ifndef LUIGI_SWITCHER_H_
#define LUIGI_SWITCHER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


typedef struct UISwitcher {
    UIElement  e;
    UIElement *active;
} UISwitcher;


UISwitcher *UISwitcherCreate(UIElement *parent, uint32_t flags);
void        UISwitcherSwitchTo(UISwitcher *switcher, UIElement *child);


int _UISwitcherMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif

#endif // LUIGI_SWITCHER_H_
