#ifndef LUIGI_UTILS_H_
#define LUIGI_UTILS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#if defined(UI_LINUX) || defined(UI_COCOA)
# include <assert.h>
# include <math.h>
# include <stdlib.h>
# include <string.h>

# define UI_ASSERT    assert
# define UI_CALLOC(x) calloc(1, (x))
# define UI_FREE      free
# define UI_MALLOC    malloc
# define UI_REALLOC   realloc
# define UI_MEMMOVE(d, s, n)                                                                       \
     do {                                                                                          \
         size_t _n = n;                                                                            \
         if (_n) {                                                                                 \
             memmove(d, s, _n);                                                                    \
         }                                                                                         \
     } while (0)
#endif

#ifdef UI_WINDOWS
# undef _UNICODE
# undef UNICODE
# include <windows.h>

extern HANDLE win_heap;

# define UI_ASSERT(x)                                                                              \
     do {                                                                                          \
         if (!(x)) {                                                                               \
             ui.platform->assertionFailure = true;                                                 \
             MessageBox(0, "Assertion failure on line " _UI_TO_STRING_2(__LINE__), 0, 0);          \
             ExitProcess(1);                                                                       \
         }                                                                                         \
     } while (0)
# define UI_CALLOC(x) HeapAlloc(win_heap, HEAP_ZERO_MEMORY, (x))
# define UI_FREE(x)   HeapFree(win_heap, 0, (x))
# define UI_MALLOC(x) HeapAlloc(win_heap, 0, (x))
# define UI_REALLOC   _UIHeapReAlloc
# define UI_MEMMOVE   _UIMemmove
#endif

#if defined(UI_ESSENCE)
# include <essence.h>

# define UI_ASSERT        EsAssert
# define UI_CALLOC(x)     EsHeapAllocate((x), true)
# define UI_FREE          EsHeapFree
# define UI_MALLOC(x)     EsHeapAllocate((x), false)
# define UI_REALLOC(x, y) EsHeapReallocate((x), (y), false)
# define UI_MEMMOVE       EsCRTmemmove

// Callback to allow the application to process messages.
void _UIMessageProcess(EsMessage *message);
#endif


#define _UI_TO_STRING_1(x) #x
#define _UI_TO_STRING_2(x) _UI_TO_STRING_1(x)


#define UI_SWAP(s, a, b)                                                                           \
    do {                                                                                           \
        s t = (a);                                                                                 \
        (a) = (b);                                                                                 \
        (b) = t;                                                                                   \
    } while (0)


#define UI_COLOR_ALPHA_F(x) ((((x) >> 24) & 0xFF) / 255.0f)
#define UI_COLOR_RED_F(x)   ((((x) >> 16) & 0xFF) / 255.0f)
#define UI_COLOR_GREEN_F(x) ((((x) >> 8) & 0xFF) / 255.0f)
#define UI_COLOR_BLUE_F(x)  ((((x) >> 0) & 0xFF) / 255.0f)
#define UI_COLOR_ALPHA(x)   ((((x) >> 24) & 0xFF))
#define UI_COLOR_RED(x)     ((((x) >> 16) & 0xFF))
#define UI_COLOR_GREEN(x)   ((((x) >> 8) & 0xFF))
#define UI_COLOR_BLUE(x)    ((((x) >> 0) & 0xFF))
#define UI_COLOR_FROM_FLOAT(r, g, b)                                                               \
    (((uint32_t)((r) * 255.0f) << 16) | ((uint32_t)((g) * 255.0f) << 8) |                          \
     ((uint32_t)((b) * 255.0f) << 0))
#define UI_COLOR_FROM_RGBA_F(r, g, b, a)                                                           \
    (((uint32_t)((r) * 255.0f) << 16) | ((uint32_t)((g) * 255.0f) << 8) |                          \
     ((uint32_t)((b) * 255.0f) << 0) | ((uint32_t)((a) * 255.0f) << 24))


extern const int UI_KEYCODE_A;
extern const int UI_KEYCODE_BACKSPACE;
extern const int UI_KEYCODE_DELETE;
extern const int UI_KEYCODE_DOWN;
extern const int UI_KEYCODE_END;
extern const int UI_KEYCODE_ENTER;
extern const int UI_KEYCODE_ESCAPE;
extern const int UI_KEYCODE_F1;
extern const int UI_KEYCODE_HOME;
extern const int UI_KEYCODE_LEFT;
extern const int UI_KEYCODE_RIGHT;
extern const int UI_KEYCODE_SPACE;
extern const int UI_KEYCODE_TAB;
extern const int UI_KEYCODE_UP;
extern const int UI_KEYCODE_INSERT;
extern const int UI_KEYCODE_0;
extern const int UI_KEYCODE_BACKTICK;
extern const int UI_KEYCODE_PAGE_UP;
extern const int UI_KEYCODE_PAGE_DOWN;


#define UI_KEYCODE_LETTER(x) (UI_KEYCODE_A + (x) - 'A')
#define UI_KEYCODE_DIGIT(x)  (UI_KEYCODE_0 + (x) - '0')
#define UI_KEYCODE_FKEY(x)   (UI_KEYCODE_F1 + (x) - 1)


ptrdiff_t _UIStringLength(const char *cString)
{
    if (!cString)
        return 0;
    ptrdiff_t length;
    for (length = 0; cString[length]; length++)
        ;
    return length;
}


#ifdef UI_UNICODE

# ifndef UI_FREETYPE
#  error "Unicode support requires Freetype"
# endif

# define _UNICODE_MAX_CODEPOINT 0x10FFFF

int Utf8GetCodePoint(const char *cString, ptrdiff_t bytesLength, ptrdiff_t *bytesConsumed)
{
    UI_ASSERT(bytesLength > 0 && "Attempted to get UTF-8 code point from an empty string");

    if (bytesConsumed == NULL) {
        ptrdiff_t bytesConsumed;
        return Utf8GetCodePoint(cString, bytesLength, &bytesConsumed);
    }

    ptrdiff_t numExtraBytes;
    uint8_t   first = cString[0];

    *bytesConsumed = 1;
    if ((first & 0xF0) == 0xF0) {
        numExtraBytes = 3;
    } else if ((first & 0xE0) == 0xE0) {
        numExtraBytes = 2;
    } else if ((first & 0xC0) == 0xC0) {
        numExtraBytes = 1;
    } else if (first & 0x7F) {
        return first & 0x80 ? -1 : first;
    } else {
        return -1;
    }

    if (bytesLength < numExtraBytes + 1) {
        return -1;
    }

    int codePoint = ((int)first & (0x3F >> numExtraBytes)) << (6 * numExtraBytes);
    for (ptrdiff_t idx = 1; idx < numExtraBytes + 1; idx++) {
        char byte = cString[idx];
        if ((byte & 0xC0) != 0x80) {
            return -1;
        }

        codePoint |= (byte & 0x3F) << (6 * (numExtraBytes - idx));
        (*bytesConsumed)++;
    }

    return codePoint > _UNICODE_MAX_CODEPOINT ? -1 : codePoint;
}

char *Utf8GetPreviousChar(char *string, char *offset)
{
    if (string == offset) {
        return string;
    }

    char *prev = offset - 1;
    while (prev > string) {
        if ((*prev & 0xC0) == 0x80)
            prev--;
        else
            break;
    }

    return prev;
}

ptrdiff_t Utf8GetCharBytes(const char *cString, ptrdiff_t bytes)
{
    if (!cString) {
        return 0;
    }
    if (bytes == -1) {
        bytes = _UIStringLength(cString);
    }

    ptrdiff_t bytesConsumed;
    Utf8GetCodePoint(cString, bytes, &bytesConsumed);
    return bytesConsumed;
}

ptrdiff_t Utf8StringLength(const char *cString, ptrdiff_t bytes)
{
    if (!cString) {
        return 0;
    }
    if (bytes == -1) {
        bytes = _UIStringLength(cString);
    }

    ptrdiff_t length    = 0;
    ptrdiff_t byteIndex = 0;
    while (byteIndex < bytes) {
        ptrdiff_t bytesConsumed;
        Utf8GetCodePoint(cString + byteIndex, bytes - byteIndex, &bytesConsumed);
        byteIndex += bytesConsumed;
        length++;

        UI_ASSERT(byteIndex <= bytes &&
                  "Overran the end of the string while counting the number of UTF-8 code points");
    }

    return length;
}

# define _UI_ADVANCE_CHAR(index, text, count) index += Utf8GetCharBytes(text, count - index)

# define _UI_SKIP_TAB(ti, text, bytesLeft, tabSize)                                                \
     do {                                                                                          \
         int c = Utf8GetCodePoint(text, bytesLeft, NULL);                                          \
         if (c == '\t')                                                                            \
             while (ti % tabSize)                                                                  \
                 ti++;                                                                             \
     } while (0)

# define _UI_MOVE_CARET_BACKWARD(caret, text, offset, offset2)                                     \
     do {                                                                                          \
         char *prev = Utf8GetPreviousChar(text, text + offset);                                    \
         caret      = prev - text - offset2;                                                       \
     } while (0)

# define _UI_MOVE_CARET_FORWARD(caret, text, bytes, offset)                                        \
     do {                                                                                          \
         caret += Utf8GetCharBytes(text + caret, bytes - offset);                                  \
     } while (0)

# define _UI_MOVE_CARET_BY_WORD(text, bytes, offset)                                               \
     {                                                                                             \
         char *prev = Utf8GetPreviousChar(text, text + offset);                                    \
         int   c1   = Utf8GetCodePoint(prev, bytes - (prev - text), NULL);                         \
         int   c2   = Utf8GetCodePoint(text + offset, bytes - offset, NULL);                       \
         if (_UICharIsAlphaOrDigitOrUnderscore(c1) != _UICharIsAlphaOrDigitOrUnderscore(c2))       \
             break;                                                                                \
     }

#else

# define _UI_ADVANCE_CHAR(index, code, count) index++

# define _UI_SKIP_TAB(ti, text, bytesLeft, tabSize)                                                \
     if (*(text) == '\t')                                                                          \
         while (ti % tabSize)                                                                      \
     ti++

# define _UI_MOVE_CARET_BACKWARD(caret, text, offset, offset2) caret--
# define _UI_MOVE_CARET_FORWARD(caret, text, bytes, offset)    caret++

# define _UI_MOVE_CARET_BY_WORD(text, bytes, offset)                                               \
     {                                                                                             \
         char c1 = (text)[offset - 1];                                                             \
         char c2 = (text)[offset];                                                                 \
         if (_UICharIsAlphaOrDigitOrUnderscore(c1) != _UICharIsAlphaOrDigitOrUnderscore(c2))       \
             break;                                                                                \
     }

#endif // UI_UNICODE


#ifdef UI_IMPLEMENTATION


typedef union _UIConvertFloatInteger {
    float    f;
    uint32_t i;
} _UIConvertFloatInteger;


float _UIFloorFloat(float x)
{
    _UIConvertFloatInteger convert  = {x};
    uint32_t               sign     = convert.i & 0x80000000;
    int                    exponent = (int)((convert.i >> 23) & 0xFF) - 0x7F;

    if (exponent >= 23) {
        // There aren't any bits representing a fractional part.
    } else if (exponent >= 0) {
        // Positive exponent.
        uint32_t mask = 0x7FFFFF >> exponent;
        if (!(mask & convert.i))
            return x; // Already an integer.
        if (sign)
            convert.i += mask;
        convert.i &= ~mask; // Mask out the fractional bits.
    } else if (exponent < 0) {
        // Negative exponent.
        return sign ? -1.0 : 0.0;
    }

    return convert.f;
}


float _UILinearMap(float value, float inFrom, float inTo, float outFrom, float outTo)
{
    float inRange = inTo - inFrom, outRange = outTo - outFrom;
    float normalisedValue = (value - inFrom) / inRange;
    return normalisedValue * outRange + outFrom;
}


bool UIColorToHSV(uint32_t rgb, float *hue, float *saturation, float *value)
{
    float r = UI_COLOR_RED_F(rgb);
    float g = UI_COLOR_GREEN_F(rgb);
    float b = UI_COLOR_BLUE_F(rgb);

    float maximum = (r > g && r > b) ? r : (g > b ? g : b),
          minimum = (r < g && r < b) ? r : (g < b ? g : b), difference = maximum - minimum;
    *value = maximum;

    if (!difference) {
        *saturation = 0;
        return false;
    } else {
        if (r == maximum)
            *hue = (g - b) / difference + 0;
        if (g == maximum)
            *hue = (b - r) / difference + 2;
        if (b == maximum)
            *hue = (r - g) / difference + 4;
        if (*hue < 0)
            *hue += 6;
        *saturation = difference / maximum;
        return true;
    }
}

void UIColorToRGB(float h, float s, float v, uint32_t *rgb)
{
    float r, g, b;

    if (!s) {
        r = g = b = v;
    } else {
        int   h0 = ((int)h) % 6;
        float f  = h - _UIFloorFloat(h);
        float x = v * (1 - s), y = v * (1 - s * f), z = v * (1 - s * (1 - f));

        switch (h0) {
        case 0:
            r = v, g = z, b = x;
            break;
        case 1:
            r = y, g = v, b = x;
            break;
        case 2:
            r = x, g = v, b = z;
            break;
        case 3:
            r = x, g = y, b = v;
            break;
        case 4:
            r = z, g = x, b = v;
            break;
        default:
            r = v, g = x, b = y;
            break;
        }
    }

    *rgb = UI_COLOR_FROM_FLOAT(r, g, b);
}


char *UIStringCopy(const char *in, ptrdiff_t inBytes)
{
    if (inBytes == -1) {
        inBytes = _UIStringLength(in);
    }

    char *buffer = (char *)UI_MALLOC(inBytes + 1);

    for (intptr_t i = 0; i < inBytes; i++) {
        buffer[i] = in[i];
    }

    buffer[inBytes] = 0;
    return buffer;
}

int _UIByteToColumn(const char *string, int byte, int bytes, int tabSize)
{
    int ti = 0, i = 0;

    while (i < byte && i < bytes) {
        ti++;
        _UI_SKIP_TAB(ti, string + i, bytes - i, tabSize);
        _UI_ADVANCE_CHAR(i, string + i, byte);
    }

    return ti;
}

int _UIColumnToByte(const char *string, int column, int bytes, int tabSize)
{
    int byte = 0, ti = 0;

    while (byte < bytes) {
        ti++;
        _UI_SKIP_TAB(ti, string + byte, bytes - byte, tabSize);
        if (column < ti)
            break;

        _UI_ADVANCE_CHAR(byte, string + byte, bytes);
    }

    return byte;
}


#endif // UI_IMPLEMENTATION


#ifdef __cplusplus
}
#endif

#endif // LUIGI_UTILS_H_
