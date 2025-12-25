#include "ui_theme.h"


/////////////////////////////////////////
// Themes.
/////////////////////////////////////////


UITheme uiThemeClassic = {
    .panel1   = 0xFFF0F0F0,
    .panel2   = 0xFFFFFFFF,
    .selected = 0xFF94BEFE,
    .border   = 0xFF404040,

    .text         = 0xFF000000,
    .textDisabled = 0xFF404040,
    .textSelected = 0xFF000000,

    .buttonNormal   = 0xFFE0E0E0,
    .buttonHovered  = 0xFFF0F0F0,
    .buttonPressed  = 0xFFA0A0A0,
    .buttonDisabled = 0xFFF0F0F0,

    .textboxNormal  = 0xFFF8F8F8,
    .textboxFocused = 0xFFFFFFFF,

    .codeFocused      = 0xFFE0E0E0,
    .codeBackground   = 0xFFFFFFFF,
    .codeDefault      = 0xFF000000,
    .codeComment      = 0xFFA11F20,
    .codeString       = 0xFF037E01,
    .codeNumber       = 0xFF213EF1,
    .codeOperator     = 0xFF7F0480,
    .codePreprocessor = 0xFF545D70,

    .accent1 = 0xFF0000,
    .accent2 = 0x00FF00,
};

UITheme uiThemeDark = {
    .panel1   = 0xFF252B31,
    .panel2   = 0xFF14181E,
    .selected = 0xFF94BEFE,
    .border   = 0xFF000000,

    .text         = 0xFFFFFFFF,
    .textDisabled = 0xFF787D81,
    .textSelected = 0xFF000000,

    .buttonNormal   = 0xFF383D41,
    .buttonHovered  = 0xFF4B5874,
    .buttonPressed  = 0xFF0D0D0F,
    .buttonDisabled = 0xFF1B1F23,

    .textboxNormal  = 0xFF31353C,
    .textboxFocused = 0xFF4D4D59,

    .codeFocused      = 0xFF505055,
    .codeBackground   = 0xFF212126,
    .codeDefault      = 0xFFFFFFFF,
    .codeComment      = 0xFFB4B4B4,
    .codeString       = 0xFFF5DDD1,
    .codeNumber       = 0xFFC3F5D3,
    .codeOperator     = 0xFFF5D499,
    .codePreprocessor = 0xFFF5F3D1,

    .accent1 = 0xF01231,
    .accent2 = 0x45F94E,
};
