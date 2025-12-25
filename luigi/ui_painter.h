#ifndef LUIGI_PAINTER_H_
#define LUIGI_PAINTER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_rect.h"
#include <stddef.h>
#include <stdint.h>


typedef struct UIPainter {
    UIRectangle clip;
    uint32_t   *bits;
    int         width, height;
#ifdef UI_DEBUG
    int fillCount;
#endif
} UIPainter;


#ifdef __cplusplus
}
#endif

#endif // LUIGI_PAINTER_H_
