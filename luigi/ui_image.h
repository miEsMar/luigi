#ifndef LUIGI_IMAGE_H_
#define LUIGI_IMAGE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


#define UI_IMAGE_DISPLAY_INTERACTIVE        (1 << 0)
#define _UI_IMAGE_DISPLAY_ZOOM_FIT          (1 << 1)
#define UI_IMAGE_DISPLAY_MIDDLE_DRAG_TO_PAN (1 << 2)
#define UI_IMAGE_DISPLAY_HQ_ZOOM_IN         (1 << 3)


typedef struct UIImageDisplay {
    UIElement e;
    uint32_t *bits;
    int       width, height;
    float     panX, panY, zoom;

    // Internals:
    int previousWidth, previousHeight;
    int previousPanPointX, previousPanPointY;
} UIImageDisplay;


//


UIImageDisplay *UIImageDisplayCreate(UIElement *parent, uint32_t flags, uint32_t *bits,
                                     size_t width, size_t height, size_t stride);
void UIImageDisplaySetContent(UIImageDisplay *display, uint32_t *bits, size_t width, size_t height,
                              size_t stride);


void _UIImageDisplayUpdateViewport(UIImageDisplay *display);
int  _UIImageDisplayMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_IMAGE_H_
