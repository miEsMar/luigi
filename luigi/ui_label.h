#ifndef LUIGI_LABEL_H_
#define LUIGI_LABEL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


typedef struct UILabel {
    UIElement e;
    char     *label;
    ptrdiff_t labelBytes;
} UILabel;


//


UILabel *UILabelCreate(UIElement *parent, uint32_t flags, const char *string,
                       ptrdiff_t stringBytes);
void     UILabelSetContent(UILabel *label, const char *string, ptrdiff_t stringBytes);


int _UILabelMessage(UIElement *element, UIMessage message, int di, void *dp);


#ifdef __cplusplus
}
#endif


#endif // LUIGI_LABEL_H_
