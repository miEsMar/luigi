#ifndef LUIGI_SPACER_H_
#define LUIGI_SPACER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


typedef struct UISpacer {
    UIElement e;
    int       width, height;
} UISpacer;


UISpacer *UISpacerCreate(UIElement *parent, uint32_t flags, int width, int height);


int _UISpacerMessage(UIElement *element, UIMessage message, int di, void *dp);

#ifdef __cplusplus
}
#endif

#endif // LUIGI_SPACER_H_
