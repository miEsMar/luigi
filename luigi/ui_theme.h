#ifndef LUIGI_THEME_H_
#define LUIGI_THEME_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef enum UIThemeItem {
    UIThemeItem_panel1,
    UIThemeItem_panel2,
    UIThemeItem_selected,
    UIThemeItem_border,
    UIThemeItem_text,
    UIThemeItem_textDisabled,
    UIThemeItem_textSelected,
    UIThemeItem_buttonNormal,
    UIThemeItem_buttonHovered,
    UIThemeItem_buttonPressed,
    UIThemeItem_buttonDisabled,
    UIThemeItem_textboxNormal,
    UIThemeItem_textboxFocused,
    UIThemeItem_codeFocused,
    UIThemeItem_codeBackground,
    UIThemeItem_codeDefault,
    UIThemeItem_codeComment,
    UIThemeItem_codeString,
    UIThemeItem_codeNumber,
    UIThemeItem_codeOperator,
    UIThemeItem_codePreprocessor,
    UIThemeItem_accent1,
    UIThemeItem_accent2,
    UIThemeItem_MAX,
} UIThemeItem;


typedef struct UITheme {
    union {
        struct {
            uint32_t panel1, panel2, selected, border;
            uint32_t text, textDisabled, textSelected;
            uint32_t buttonNormal, buttonHovered, buttonPressed, buttonDisabled;
            uint32_t textboxNormal, textboxFocused;
            uint32_t codeFocused, codeBackground, codeDefault, codeComment, codeString, codeNumber,
                codeOperator, codePreprocessor;
            uint32_t accent1, accent2;
        };
        uint32_t colors[UIThemeItem_MAX];
    };
} UITheme;


extern UITheme uiThemeClassic;
extern UITheme uiThemeDark;


#ifdef __cplusplus
}
#endif

#endif // LUIGI_THEME_H_
