#include "ui_animation.h"
#include "ui.h"
#include "utils.h"
#include <stdbool.h>


bool UIElementAnimate(UIElement *element, bool stop)
{
    if (stop) {
        for (uint32_t i = 0; i < ui.animatingCount; i++) {
            if (ui.animating[i] == element) {
                ui.animating[i] = ui.animating[ui.animatingCount - 1];
                ui.animatingCount--;
                return true;
            }
        }

        return false;
    } else {
        for (uint32_t i = 0; i < ui.animatingCount; i++) {
            if (ui.animating[i] == element) {
                return true;
            }
        }

        ui.animating =
            (UIElement **)UI_REALLOC(ui.animating, sizeof(UIElement *) * (ui.animatingCount + 1));
        ui.animating[ui.animatingCount] = element;
        ui.animatingCount++;
        UI_ASSERT(~element->flags & UI_ELEMENT_DESTROY);
        return true;
    }
}


uint64_t UIAnimateClock(void)
{
    // c
    return (uint64_t)UI_CLOCK() * 1000 / UI_CLOCKS_PER_SECOND;
}

void _UIProcessAnimations(void)
{
    bool update = ui.animatingCount;

    for (uint32_t i = 0; i < ui.animatingCount; i++) {
        UIElementMessage(ui.animating[i], UI_MSG_ANIMATE, 0, 0);
    }

    if (update) {
        _UIUpdate();
    }
}
