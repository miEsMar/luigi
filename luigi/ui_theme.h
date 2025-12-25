#ifndef LUIGI_THEME_H_
#define LUIGI_THEME_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef struct UITheme {
    uint32_t panel1, panel2, selected, border;
    uint32_t text, textDisabled, textSelected;
    uint32_t buttonNormal, buttonHovered, buttonPressed, buttonDisabled;
    uint32_t textboxNormal, textboxFocused;
    uint32_t codeFocused, codeBackground, codeDefault, codeComment, codeString, codeNumber,
        codeOperator, codePreprocessor;
    uint32_t accent1, accent2;
} UITheme;


extern UITheme uiThemeClassic;
extern UITheme uiThemeDark;


#ifdef __cplusplus
}
#endif

#endif // LUIGI_THEME_H_
