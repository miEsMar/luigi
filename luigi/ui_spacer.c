#include "ui_spacer.h"
#include "ui_window.h"


/////////////////////////////////////////
// Spacers.
/////////////////////////////////////////

int _UISpacerMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UISpacer *spacer = (UISpacer *)element;

    if (message == UI_MSG_GET_HEIGHT) {
        return spacer->height * element->window->scale;
    } else if (message == UI_MSG_GET_WIDTH) {
        return spacer->width * element->window->scale;
    }

    return 0;
}

UISpacer *UISpacerCreate(UIElement *parent, uint32_t flags, int width, int height)
{
    UISpacer *spacer =
        (UISpacer *)UIElementCreate(sizeof(UISpacer), parent, flags, _UISpacerMessage, "Spacer");
    spacer->width  = width;
    spacer->height = height;
    return spacer;
}
