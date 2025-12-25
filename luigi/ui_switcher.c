#include "ui_switcher.h"
#include "ui_element.h"
#include "utils.h"


int _UISwitcherMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UISwitcher *switcher = (UISwitcher *)element;

    if (!switcher->active) {
    } else if (message == UI_MSG_GET_WIDTH || message == UI_MSG_GET_HEIGHT) {
        return UIElementMessage(switcher->active, message, di, dp);
    } else if (message == UI_MSG_LAYOUT) {
        UIElementMove(switcher->active, element->bounds, false);
    }

    return 0;
}


void UISwitcherSwitchTo(UISwitcher *switcher, UIElement *child)
{
    for (uint32_t i = 0; i < switcher->e.childCount; i++) {
        switcher->e.children[i]->flags |= UI_ELEMENT_HIDE;
    }

    UI_ASSERT(child->parent == &switcher->e);
    child->flags &= ~UI_ELEMENT_HIDE;
    switcher->active = child;
    UIElementMeasurementsChanged(&switcher->e, 3);
    UIElementRefresh(&switcher->e);
}


UISwitcher *UISwitcherCreate(UIElement *parent, uint32_t flags)
{
    return (UISwitcher *)UIElementCreate(sizeof(UISwitcher), parent, flags, _UISwitcherMessage,
                                         "Switcher");
}
