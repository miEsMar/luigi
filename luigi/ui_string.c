#include "ui_string.h"
#include "ui.h"
#include "utils.h"


int UIMeasureStringWidth(const char *string, ptrdiff_t bytes)
{
#ifdef UI_UNICODE
    return Utf8StringLength(string, bytes) * ui.activeFont->glyphWidth;
#else
    if (bytes == -1) {
        bytes = _UIStringLength(string);
    }

    return bytes * ui.activeFont->glyphWidth;
#endif
}

int UIMeasureStringHeight(void)
{
    // c
    return ui.activeFont->glyphHeight;
}
