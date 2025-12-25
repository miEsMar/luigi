#include "../platform.h"
#include "../ui.h"
#include "../ui_window.h"
#include "../utils.h"


//


void _UIClipboardWriteText(UIWindow *window, char *text)
{
    UI_FREE(ui.platform->pasteText);
    ui.platform->pasteText = text;
    XSetSelectionOwner(ui.platform->display, ui.platform->clipboardID, window->window.window, 0);
}


char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    Window clipboardOwner = XGetSelectionOwner(ui.platform->display, ui.platform->clipboardID);

    if (clipboardOwner == None) {
        return NULL;
    }

    if (_UIFindWindow(clipboardOwner)) {
        *bytes     = strlen(ui.platform->pasteText);
        char *copy = (char *)UI_MALLOC(*bytes);
        memcpy(copy, ui.platform->pasteText, *bytes);
        return copy;
    }

    XConvertSelection(ui.platform->display, ui.platform->clipboardID, XA_STRING,
                      ui.platform->xSelectionDataID, window->window.window, CurrentTime);
    XSync(ui.platform->display, 0);
    XNextEvent(ui.platform->display, &ui.platform->copyEvent);

    // Hack to get around the fact that PropertyNotify arrives before SelectionNotify.
    // We need PropertyNotify for incremental transfers.
    while (ui.platform->copyEvent.type == PropertyNotify) {
        XNextEvent(ui.platform->display, &ui.platform->copyEvent);
    }

    if (ui.platform->copyEvent.type == SelectionNotify &&
        ui.platform->copyEvent.xselection.selection == ui.platform->clipboardID &&
        ui.platform->copyEvent.xselection.property) {
        Atom target;
        // This `itemAmount` is actually `bytes_after_return`
        unsigned long size, itemAmount;
        char         *data;
        int           format;
        XGetWindowProperty(ui.platform->copyEvent.xselection.display,
                           ui.platform->copyEvent.xselection.requestor,
                           ui.platform->copyEvent.xselection.property, 0L, ~0L, 0, AnyPropertyType,
                           &target, &format, &size, &itemAmount, (unsigned char **)&data);

        // We have to allocate for incremental transfers but we don't have to allocate for
        // non-incremental transfers. I'm allocating for both here to make _UIClipboardReadTextEnd
        // work the same for both
        if (target != ui.platform->incrID) {
            *bytes     = size;
            char *copy = (char *)UI_MALLOC(*bytes);
            memcpy(copy, data, *bytes);
            XFree(data);
            XDeleteProperty(ui.platform->copyEvent.xselection.display,
                            ui.platform->copyEvent.xselection.requestor,
                            ui.platform->copyEvent.xselection.property);
            return copy;
        }

        XFree(data);
        XDeleteProperty(ui.platform->display, ui.platform->copyEvent.xselection.requestor,
                        ui.platform->copyEvent.xselection.property);
        XSync(ui.platform->display, 0);

        *bytes         = 0;
        char *fullData = NULL;

        while (true) {
            // TODO Timeout.
            XNextEvent(ui.platform->display, &ui.platform->copyEvent);

            if (ui.platform->copyEvent.type == PropertyNotify) {
                // The other case - PropertyDelete would be caused by us and can be ignored
                if (ui.platform->copyEvent.xproperty.state == PropertyNewValue) {
                    unsigned long chunkSize;

                    // Note that this call deletes the property.
                    XGetWindowProperty(
                        ui.platform->display, ui.platform->copyEvent.xproperty.window,
                        ui.platform->copyEvent.xproperty.atom, 0L, ~0L, True, AnyPropertyType,
                        &target, &format, &chunkSize, &itemAmount, (unsigned char **)&data);

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
