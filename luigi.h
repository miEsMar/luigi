// TODO UITextbox features - mouse input, undo, number dragging.
// TODO New elements - list view, menu bar.
// TODO Keyboard navigation in menus.
// TODO Easier to use fonts.

#ifndef LUIGI_H_
#define LUIGI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "luigi/automation_tests.h"
#include "luigi/core.h"
#include "luigi/font.h"
#include "luigi/inspector.h"
#include "luigi/mdi.h"
#include "luigi/timing.h"
#include "luigi/ui.h"
#include "luigi/ui_animation.h"
#include "luigi/ui_button.h"
#include "luigi/ui_checkbox.h"
#include "luigi/ui_clipboard.h"
#include "luigi/ui_code.h"
#include "luigi/ui_color.h"
#include "luigi/ui_dialog.h"
#include "luigi/ui_draw.h"
#include "luigi/ui_element.h"
#include "luigi/ui_event.h"
#include "luigi/ui_gauge.h"
#include "luigi/ui_image.h"
#include "luigi/ui_key.h"
#include "luigi/ui_label.h"
#include "luigi/ui_menu.h"
#include "luigi/ui_painter.h"
#include "luigi/ui_pane.h"
#include "luigi/ui_panel.h"
#include "luigi/ui_rect.h"
#include "luigi/ui_scroll.h"
#include "luigi/ui_shortcut.h"
#include "luigi/ui_slider.h"
#include "luigi/ui_spacer.h"
#include "luigi/ui_string.h"
#include "luigi/ui_switcher.h"
#include "luigi/ui_table.h"
#include "luigi/ui_textbox.h"
#include "luigi/ui_theme.h"
#include "luigi/ui_window.h"
#include "luigi/utils.h"


#ifdef UI_IMPLEMENTATION

# include "luigi/font.c"
# include "luigi/inspector.c"
# include "luigi/mdi.c"
# include "luigi/ui.c"
# include "luigi/ui_animation.c"
# include "luigi/ui_button.c"
# include "luigi/ui_checkbox.c"
# include "luigi/ui_clipboard.c"
# include "luigi/ui_code.c"
# include "luigi/ui_dialog.c"
# include "luigi/ui_draw.c"
# include "luigi/ui_element.c"
# include "luigi/ui_event.c"
# include "luigi/ui_gauge.c"
# include "luigi/ui_image.c"
# include "luigi/ui_label.c"
# include "luigi/ui_menu.c"
# include "luigi/ui_pane.c"
# include "luigi/ui_panel.c"
# include "luigi/ui_rect.c"
# include "luigi/ui_scroll.c"
# include "luigi/ui_slider.c"
# include "luigi/ui_spacer.c"
# include "luigi/ui_string.c"
# include "luigi/ui_switcher.c"
# include "luigi/ui_table.c"
# include "luigi/ui_textbox.c"
# include "luigi/ui_theme.c"
# include "luigi/ui_window.c"

#endif


#ifdef __cplusplus
}
#endif


#endif // LUIGI_H_
