#include "ui.h"
#include "ui_window.h"
#include "utils.h"


void _UIClipboardWriteText(UIWindow *window, char *text)
{
    UI_FREE(ui.pasteText);
    ui.pasteText = text;
    XSetSelectionOwner(ui.display, ui.clipboardID, window->window, 0);
}


char *_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    Window clipboardOwner = XGetSelectionOwner(ui.display, ui.clipboardID);

    if (clipboardOwner == None) {
        return NULL;
    }

    if (_UIFindWindow(clipboardOwner)) {
        *bytes     = strlen(ui.pasteText);
        char *copy = (char *)UI_MALLOC(*bytes);
        memcpy(copy, ui.pasteText, *bytes);
        return copy;
    }

    XConvertSelection(ui.display, ui.clipboardID, XA_STRING, ui.xSelectionDataID, window->window,
                      CurrentTime);
    XSync(ui.display, 0);
    XNextEvent(ui.display, &ui.copyEvent);

    // Hack to get around the fact that PropertyNotify arrives before SelectionNotify.
    // We need PropertyNotify for incremental transfers.
    while (ui.copyEvent.type == PropertyNotify) {
        XNextEvent(ui.display, &ui.copyEvent);
    }

    if (ui.copyEvent.type == SelectionNotify &&
        ui.copyEvent.xselection.selection == ui.clipboardID && ui.copyEvent.xselection.property) {
        Atom target;
        // This `itemAmount` is actually `bytes_after_return`
        unsigned long size, itemAmount;
        char         *data;
        int           format;
        XGetWindowProperty(ui.copyEvent.xselection.display, ui.copyEvent.xselection.requestor,
                           ui.copyEvent.xselection.property, 0L, ~0L, 0, AnyPropertyType, &target,
                           &format, &size, &itemAmount, (unsigned char **)&data);

        // We have to allocate for incremental transfers but we don't have to allocate for
        // non-incremental transfers. I'm allocating for both here to make _UIClipboardReadTextEnd
        // work the same for both
        if (target != ui.incrID) {
            *bytes     = size;
            char *copy = (char *)UI_MALLOC(*bytes);
            memcpy(copy, data, *bytes);
            XFree(data);
            XDeleteProperty(ui.copyEvent.xselection.display, ui.copyEvent.xselection.requestor,
                            ui.copyEvent.xselection.property);
            return copy;
        }

        XFree(data);
        XDeleteProperty(ui.display, ui.copyEvent.xselection.requestor,
                        ui.copyEvent.xselection.property);
        XSync(ui.display, 0);

        *bytes         = 0;
        char *fullData = NULL;

        while (true) {
            // TODO Timeout.
            XNextEvent(ui.display, &ui.copyEvent);

            if (ui.copyEvent.type == PropertyNotify) {
                // The other case - PropertyDelete would be caused by us and can be ignored
                if (ui.copyEvent.xproperty.state == PropertyNewValue) {
                    unsigned long chunkSize;

                    // Note that this call deletes the property.
                    XGetWindowProperty(ui.display, ui.copyEvent.xproperty.window,
                                       ui.copyEvent.xproperty.atom, 0L, ~0L, True, AnyPropertyType,
                                       &target, &format, &chunkSize, &itemAmount,
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
