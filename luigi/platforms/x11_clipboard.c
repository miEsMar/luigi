#include "../platform.h"
#include "../ui.h"
#include "../ui_window.h"
#include "../utils.h"
#include "./x11.h"


//


void _UIClipboardWriteText(UIWindow *window, char *text)
{
    Luigi_Platform_X11 *platform = ui.platform;

    UI_FREE(platform->pasteText);
    platform->pasteText = text;
    XSetSelectionOwner(platform->display, platform->clipboardID, window->window.window, 0);
}


char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    Luigi_Platform_X11 *platform = ui.platform;

    Window clipboardOwner = XGetSelectionOwner(platform->display, platform->clipboardID);

    if (clipboardOwner == None) {
        return NULL;
    }

    if (_UIFindWindow(clipboardOwner)) {
        *bytes     = strlen(platform->pasteText);
        char *copy = (char *)UI_MALLOC(*bytes);
        memcpy(copy, platform->pasteText, *bytes);
        return copy;
    }

    XConvertSelection(platform->display, platform->clipboardID, XA_STRING,
                      platform->xSelectionDataID, window->window.window, CurrentTime);
    XSync(platform->display, 0);
    XNextEvent(platform->display, &platform->copyEvent);

    // Hack to get around the fact that PropertyNotify arrives before SelectionNotify.
    // We need PropertyNotify for incremental transfers.
    while (platform->copyEvent.type == PropertyNotify) {
        XNextEvent(platform->display, &platform->copyEvent);
    }

    if (platform->copyEvent.type == SelectionNotify &&
        platform->copyEvent.xselection.selection == platform->clipboardID &&
        platform->copyEvent.xselection.property) {
        Atom target;
        // This `itemAmount` is actually `bytes_after_return`
        unsigned long size, itemAmount;
        char         *data;
        int           format;
        XGetWindowProperty(platform->copyEvent.xselection.display,
                           platform->copyEvent.xselection.requestor,
                           platform->copyEvent.xselection.property, 0L, ~0L, 0, AnyPropertyType,
                           &target, &format, &size, &itemAmount, (unsigned char **)&data);

        // We have to allocate for incremental transfers but we don't have to allocate for
        // non-incremental transfers. I'm allocating for both here to make _UIClipboardReadTextEnd
        // work the same for both
        if (target != platform->incrID) {
            *bytes     = size;
            char *copy = (char *)UI_MALLOC(*bytes);
            memcpy(copy, data, *bytes);
            XFree(data);
            XDeleteProperty(platform->copyEvent.xselection.display,
                            platform->copyEvent.xselection.requestor,
                            platform->copyEvent.xselection.property);
            return copy;
        }

        XFree(data);
        XDeleteProperty(platform->display, platform->copyEvent.xselection.requestor,
                        platform->copyEvent.xselection.property);
        XSync(platform->display, 0);

        *bytes         = 0;
        char *fullData = NULL;

        while (true) {
            // TODO Timeout.
            XNextEvent(platform->display, &platform->copyEvent);

            if (platform->copyEvent.type == PropertyNotify) {
                // The other case - PropertyDelete would be caused by us and can be ignored
                if (platform->copyEvent.xproperty.state == PropertyNewValue) {
                    unsigned long chunkSize;

                    // Note that this call deletes the property.
                    XGetWindowProperty(platform->display, platform->copyEvent.xproperty.window,
                                       platform->copyEvent.xproperty.atom, 0L, ~0L, True,
                                       AnyPropertyType, &target, &format, &chunkSize, &itemAmount,
                                       (unsigned char **)&data);

                    if (chunkSize == 0) {
                        return fullData;
                    } else {
                        ptrdiff_t currentOffset = *bytes;
                        *bytes += chunkSize;
                        fullData = (char *)UI_REALLOC(fullData, *bytes);
                        memcpy(fullData + currentOffset, data, chunkSize);
                    }

                    XFree(data);
                }
            }
        }
    } else {
        // TODO What should happen in this case? Is the next event always going to be the selection
        // event?
        return NULL;
    }
}

void _UIClipboardReadTextEnd(UIWindow *window, char *text)
{
    if (text) {
        UI_FREE(text);
    }
}
