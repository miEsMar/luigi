#include "ui_element.h"
#include "ui.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_rect.h"
#include "utils.h"

#ifdef UI_DEBUG
# include "inspector.h"
#endif


UIElement *UIElementFindByPoint(UIElement *element, int x, int y)
{
    for (uint32_t i = element->childCount; i > 0; i--) {
        UIElement *child = element->children[i - 1];

        if ((~child->flags & UI_ELEMENT_HIDE) && UIRectangleContains(child->clip, x, y)) {
            return UIElementFindByPoint(child, x, y);
        }
    }

    return element;
}


//


UIElement *_UIElementNextOrPreviousSibling(UIElement *element, bool previous)
{
    if (!element->parent) {
        return NULL;
    }

    for (uint32_t i = 0; i < element->parent->childCount; i++) {
        if (element->parent->children[i] == element) {
            if (previous) {
                return i > 0 ? element->parent->children[i - 1] : NULL;
            } else {
                return i < element->parent->childCount - 1 ? element->parent->children[i + 1]
                                                           : NULL;
            }
        }
    }

    UI_ASSERT(false);
    return NULL;
}


void _UIElementDestroyDescendents(UIElement *element, bool topLevel)
{
    for (uint32_t i = 0; i < element->childCount; i++) {
        UIElement *child = element->children[i];

        if (!topLevel || (~child->flags & UI_ELEMENT_NON_CLIENT)) {
            UIElementDestroy(child);
        }
    }

#ifdef UI_DEBUG
    _UIInspectorRefresh();
#endif
}


void UIElementDestroyDescendents(UIElement *element)
{
    _UIElementDestroyDescendents(element, true);
}


void UIElementDestroy(UIElement *element)
{
    if (element->flags & UI_ELEMENT_DESTROY) {
        return;
    }

    UIElementMessage(element, UI_MSG_DESTROY, 0, 0);
    element->flags |= UI_ELEMENT_DESTROY | UI_ELEMENT_HIDE;

    UIElement *ancestor = element->parent;

    while (ancestor) {
        if (ancestor->flags & UI_ELEMENT_DESTROY_DESCENDENT)
            break;
        ancestor->flags |= UI_ELEMENT_DESTROY_DESCENDENT;
        ancestor = ancestor->parent;
    }

    _UIElementDestroyDescendents(element, false);

    if (element->parent) {
        UIElementRelayout(element->parent);
        UIElementRepaint(element->parent, &element->bounds);
        UIElementMeasurementsChanged(element->parent, 3);
    }
}

int UIElementMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    if (message != UI_MSG_DEALLOCATE && (element->flags & UI_ELEMENT_DESTROY)) {
        return 0;
    }

    if (message >= UI_MSG_INPUT_EVENTS_START && message <= UI_MSG_INPUT_EVENTS_END &&
        (element->flags & UI_ELEMENT_DISABLED)) {
        return 0;
    }

    if (element->messageUser) {
        int result = element->messageUser(element, message, di, dp);

        if (result) {
            return result;
        }
    }

    if (element->messageClass) {
        return element->messageClass(element, message, di, dp);
    } else {
        return 0;
    }
}

UIElement *UIElementChangeParent(UIElement *element, UIElement *newParent, UIElement *insertBefore)
{
    bool       found     = false;
    UIElement *oldBefore = NULL;

    for (uint32_t i = 0; i < element->parent->childCount; i++) {
        if (element->parent->children[i] == element) {
            UI_MEMMOVE(&element->parent->children[i], &element->parent->children[i + 1],
                       sizeof(UIElement *) * (element->parent->childCount - i - 1));
            element->parent->childCount--;
            oldBefore = i == element->parent->childCount ? NULL : element->parent->children[i];
            found     = true;
            break;
        }
    }

    UI_ASSERT(found && (~element->flags & UI_ELEMENT_DESTROY));

    for (uint32_t i = 0; i <= newParent->childCount; i++) {
        if (i == newParent->childCount || newParent->children[i] == insertBefore) {
            newParent->children = (UIElement **)UI_REALLOC(
                newParent->children, sizeof(UIElement *) * (newParent->childCount + 1));
            UI_MEMMOVE(&newParent->children[i + 1], &newParent->children[i],
                       sizeof(UIElement *) * (newParent->childCount - i));
            newParent->childCount++;
            newParent->children[i] = element;
            found                  = true;
            break;
        }
    }

    UIElement *oldParent = element->parent;
    element->parent      = newParent;
    element->window      = newParent->window;

    UIElementMeasurementsChanged(oldParent, 3);
    UIElementMeasurementsChanged(newParent, 3);

    return oldBefore;
}

UIElement *UIElementCreate(size_t bytes, UIElement *parent, uint32_t flags,
                           int (*message)(UIElement *, UIMessage, int, void *),
                           const char *cClassName)
{
    UI_ASSERT(bytes >= sizeof(UIElement));
    UIElement *element    = (UIElement *)UI_CALLOC(bytes);
    element->flags        = flags;
    element->messageClass = message;

    if (!parent && (~flags & UI_ELEMENT_WINDOW)) {
        UI_ASSERT(ui.parentStackCount);
        parent = ui.parentStack[ui.parentStackCount - 1];
    }

    if (parent) {
        UI_ASSERT(~parent->flags & UI_ELEMENT_DESTROY);
        element->window                      = parent->window;
        element->parent                      = parent;
        parent->children                     = (UIElement **)UI_REALLOC(parent->children,
                                                                        sizeof(UIElement *) * (parent->childCount + 1));
        parent->children[parent->childCount] = element;
        parent->childCount++;
        UIElementRelayout(parent);
        UIElementMeasurementsChanged(parent, 3);
    }

    element->cClassName = cClassName;
    static uint32_t id  = 0;
    element->id         = ++id;

#ifdef UI_DEBUG
    _UIInspectorRefresh();
#endif

    if (flags & UI_ELEMENT_PARENT_PUSH) {
        UIParentPush(element);
    }

    return element;
}


UIElement *UIParentPush(UIElement *element)
{
    UI_ASSERT(ui.parentStackCount != sizeof(ui.parentStack) / sizeof(ui.parentStack[0]));
    ui.parentStack[ui.parentStackCount++] = element;
    return element;
}

UIElement *UIParentPop(void)
{
    UI_ASSERT(ui.parentStackCount);
    ui.parentStackCount--;
    return ui.parentStack[ui.parentStackCount];
}


/////////////////////////////////////////
// Miscellaneous core functions.
/////////////////////////////////////////

UIRectangle UIElementScreenBounds(UIElement *element)
{
    int x = 0, y = 0;
    Luigi_Platform_get_screen_pos(&element->window->window, &x, &y);
    return UIRectangleAdd(element->bounds, UI_RECT_2(x, y));
}

void UIWindowRegisterShortcut(UIWindow *window, UIShortcut shortcut)
{
    if (window->shortcutCount + 1 > window->shortcutAllocated) {
        window->shortcutAllocated = (window->shortcutCount + 1) * 2;
        window->shortcuts = (UIShortcut *)UI_REALLOC(window->shortcuts, window->shortcutAllocated *
                                                                            sizeof(UIShortcut));
    }

    window->shortcuts[window->shortcutCount++] = shortcut;
}

void UIElementSetDisabled(UIElement *element, bool disabled)
{
    if (element->window->focused == element && disabled) {
        UIElementFocus(&element->window->e);
    }

    if ((element->flags & UI_ELEMENT_DISABLED) && disabled)
        return;
    if ((~element->flags & UI_ELEMENT_DISABLED) && !disabled)
        return;

    if (disabled)
        element->flags |= UI_ELEMENT_DISABLED;
    else
        element->flags &= ~UI_ELEMENT_DISABLED;

    UIElementMessage(element, UI_MSG_UPDATE, UI_UPDATE_DISABLED, 0);
}

void UIElementFocus(UIElement *element)
{
    UIElement *previous = element->window->focused;
    if (previous == element)
        return;
    element->window->focused = element;
    if (previous)
        UIElementMessage(previous, UI_MSG_UPDATE, UI_UPDATE_FOCUSED, 0);
    if (element)
        UIElementMessage(element, UI_MSG_UPDATE, UI_UPDATE_FOCUSED, 0);

#ifdef UI_DEBUG
    _UIInspectorRefresh();
#endif
}


/////////////////////////////////////////
// Update cycles.
/////////////////////////////////////////


void UIElementRefresh(UIElement *element)
{
    UIElementRelayout(element);
    UIElementRepaint(element, NULL);
}

void UIElementRelayout(UIElement *element)
{
    if (element->flags & UI_ELEMENT_RELAYOUT) {
        return;
    }

    element->flags |= UI_ELEMENT_RELAYOUT;
    UIElement *ancestor = element->parent;

    while (ancestor) {
        ancestor->flags |= UI_ELEMENT_RELAYOUT_DESCENDENT;
        ancestor = ancestor->parent;
    }
}

void UIElementMeasurementsChanged(UIElement *element, int which)
{
    if (!element->parent) {
        return; // This is the window element.
    }

    while (true) {
        if (element->parent->flags & UI_ELEMENT_DESTROY)
            return;
        which &= ~UIElementMessage(element->parent, UI_MSG_GET_CHILD_STABILITY, which, element);
        if (!which)
            break;
        element->flags |= UI_ELEMENT_RELAYOUT;
        element = element->parent;
    }

    UIElementRelayout(element);
}

void UIElementRepaint(UIElement *element, UIRectangle *region)
{
    if (!region) {
        region = &element->bounds;
    }

    UIRectangle r = UIRectangleIntersection(*region, element->clip);

    if (!UI_RECT_VALID(r)) {
        return;
    }

    if (UI_RECT_VALID(element->window->updateRegion)) {
        element->window->updateRegion = UIRectangleBounding(element->window->updateRegion, r);
    } else {
        element->window->updateRegion = r;
    }
}


void UIElementMove(UIElement *element, UIRectangle bounds, bool layout)
{
    UIRectangle clip =
        element->parent ? UIRectangleIntersection(element->parent->clip, bounds) : bounds;
    bool moved =
        !UIRectangleEquals(element->bounds, bounds) || !UIRectangleEquals(element->clip, clip);

    if (moved) {
        layout = true;

        UIElementRepaint(&element->window->e, &element->clip);
        UIElementRepaint(&element->window->e, &clip);

        element->bounds = bounds;
        element->clip   = clip;
    }

    if (element->flags & UI_ELEMENT_RELAYOUT) {
        layout = true;
    }

    if (layout) {
        UIElementMessage(element, UI_MSG_LAYOUT, 0, 0);
    } else if (element->flags & UI_ELEMENT_RELAYOUT_DESCENDENT) {
        for (uint32_t i = 0; i < element->childCount; i++) {
            UIElementMove(element->children[i], element->children[i]->bounds, false);
        }
    }

    element->flags &= ~(UI_ELEMENT_RELAYOUT_DESCENDENT | UI_ELEMENT_RELAYOUT);
}


void _UIElementPaint(UIElement *element, UIPainter *painter)
{
    if (element->flags & UI_ELEMENT_HIDE) {
        return;
    }

    // Clip painting to the element's clip.

    painter->clip = UIRectangleIntersection(element->clip, painter->clip);

    if (!UI_RECT_VALID(painter->clip)) {
        return;
    }

    // Paint the element.

    UIElementMessage(element, UI_MSG_PAINT, 0, painter);

    // Paint its children.

    UIRectangle previousClip = painter->clip;

    for (uintptr_t i = 0; i < element->childCount; i++) {
        painter->clip = previousClip;
        _UIElementPaint(element->children[i], painter);
    }

    // Draw the foreground and border.

    painter->clip = previousClip;
    UIElementMessage(element, UI_MSG_PAINT_FOREGROUND, 0, painter);

    if (element->flags & UI_ELEMENT_BORDER) {
        UIDrawBorder(painter, element->bounds, ui.theme.border,
                     UI_RECT_1((int)element->window->scale));
    }
}


bool _UIDestroy(UIElement *element)
{
    if (element->flags & UI_ELEMENT_DESTROY_DESCENDENT) {
        element->flags &= ~UI_ELEMENT_DESTROY_DESCENDENT;

        for (uintptr_t i = 0; i < element->childCount; i++) {
            if (_UIDestroy(element->children[i])) {
                UI_MEMMOVE(&element->children[i], &element->children[i + 1],
                           sizeof(UIElement *) * (element->childCount - i - 1));
                element->childCount--, i--;
            }
        }
    }

    if (element->flags & UI_ELEMENT_DESTROY) {
        UIElementMessage(element, UI_MSG_DEALLOCATE, 0, 0);

        if (element->window->pressed == element) {
            _UIWindowSetPressed(element->window, NULL, 0);
        }

        if (element->window->hovered == element) {
            element->window->hovered = &element->window->e;
        }

        if (element->window->focused == element) {
            element->window->focused = NULL;
        }

        if (element->window->dialogOldFocus == element) {
            element->window->dialogOldFocus = NULL;
        }

        UIElementAnimate(element, true);
        UI_FREE(element->children);
        UI_FREE(element);
        return true;
    } else {
        return false;
    }
}
