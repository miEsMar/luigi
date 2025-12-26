#include "ui_code.h"
#include "font.h"
#include "ui.h"
#include "ui_animation.h"
#include "ui_clipboard.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_menu.h"
#include "utils.h"
#include <stdio.h>


//


static int _UICodeMessage(UIElement *element, UIMessage message, int di, void *dp)
{
    UICode *code = (UICode *)element;

    if (message == UI_MSG_LAYOUT) {
        UIFont *previousFont   = UIFontActivate(code->font);
        int     scrollBarSize  = UI_SIZE_SCROLL_BAR * code->e.window->scale;
        code->vScroll->maximum = code->lineCount * UIMeasureStringHeight();
        code->hScroll->maximum =
            code->columns *
            code->font->glyphWidth; // TODO This doesn't take into account tab sizes!
        int vSpace = code->vScroll->page = UI_RECT_HEIGHT(element->bounds);
        int hSpace = code->hScroll->page = UI_RECT_WIDTH(element->bounds);

        if (code->moveScrollToCaretNextLayout) {
            int top     = code->selection[3].line * UIMeasureStringHeight();
            int bottom  = top + UIMeasureStringHeight();
            int context = UIMeasureStringHeight() * 2;
            if (bottom > code->vScroll->position + vSpace - context)
                code->vScroll->position = bottom - vSpace + context;
            if (top < code->vScroll->position + context)
                code->vScroll->position = top - context;
            code->moveScrollToCaretNextLayout = code->moveScrollToFocusNextLayout = false;
            // TODO Horizontal scrolling.
        } else if (code->moveScrollToFocusNextLayout) {
            code->vScroll->position = (code->focused + 0.5) * UIMeasureStringHeight() -
                                      ((float)UI_RECT_HEIGHT(code->e.bounds) / 2);
        }

        if (!(code->e.flags & UI_CODE_NO_MARGIN))
            hSpace -= UI_SIZE_CODE_MARGIN + UI_SIZE_CODE_MARGIN_GAP;
        _UI_LAYOUT_SCROLL_BAR_PAIR(code);

        UIFontActivate(previousFont);
    } else if (message == UI_MSG_PAINT) {
        UIFont *previousFont = UIFontActivate(code->font);

        UIPainter  *painter    = (UIPainter *)dp;
        UIRectangle lineBounds = element->bounds;

        lineBounds.r = code->vScroll->e.bounds.l;

        if (~code->e.flags & UI_CODE_NO_MARGIN) {
            lineBounds.l += UI_SIZE_CODE_MARGIN + UI_SIZE_CODE_MARGIN_GAP;
        }

        int lineHeight = UIMeasureStringHeight();
        lineBounds.t -= (int64_t)code->vScroll->position % lineHeight;

        UIDrawBlock(painter, element->bounds, ui.theme.codeBackground);

#ifdef __cplusplus
        UIStringSelection selection = {};
#else
        UIStringSelection selection = {0};
#endif
        selection.colorBackground = ui.theme.selected;
        selection.colorText       = ui.theme.textSelected;

        for (int i = code->vScroll->position / lineHeight; i < code->lineCount; i++) {
            if (lineBounds.t > element->clip.b) {
                break;
            }

            lineBounds.b = lineBounds.t + lineHeight;

            if (~code->e.flags & UI_CODE_NO_MARGIN) {
                char string[16];
                int  p          = 16;
                int  lineNumber = i + 1;

                while (lineNumber) {
                    string[--p] = (lineNumber % 10) + '0';
                    lineNumber /= 10;
                }

                UIRectangle marginBounds = lineBounds;
                marginBounds.r           = marginBounds.l - UI_SIZE_CODE_MARGIN_GAP;
                marginBounds.l -= UI_SIZE_CODE_MARGIN + UI_SIZE_CODE_MARGIN_GAP;

                uint32_t marginColor =
                    UIElementMessage(element, UI_MSG_CODE_GET_MARGIN_COLOR, i + 1, 0);

                if (marginColor) {
                    UIDrawBlock(painter, marginBounds, marginColor);
                }

                UIDrawString(painter, marginBounds, string + p, 16 - p,
                             marginColor ? ui.theme.codeDefault : ui.theme.codeComment,
                             UI_ALIGN_RIGHT, NULL);
            }

            if (code->focused == i) {
                UIDrawBlock(painter, lineBounds, ui.theme.codeFocused);
            }

            UIRectangle oldClip = painter->clip;
            painter->clip       = UIRectangleIntersection(oldClip, lineBounds);
            if (code->hScroll)
                lineBounds.l -= (int64_t)code->hScroll->position;
            selection.carets[0] = i == code->selection[0].line
                                      ? _UICodeByteToColumn(code, i, code->selection[0].offset)
                                      : 0;
            selection.carets[1] = i == code->selection[1].line
                                      ? _UICodeByteToColumn(code, i, code->selection[1].offset)
                                      : code->lines[i].bytes;
            int x               = UIDrawStringHighlighted(
                painter, lineBounds, code->content + code->lines[i].offset, code->lines[i].bytes,
                code->tabSize,
                element->window->focused == element && i >= code->selection[0].line &&
                        i <= code->selection[1].line
                                  ? &selection
                                  : NULL);
            int y = (lineBounds.t + lineBounds.b - UIMeasureStringHeight()) / 2;

            if (element->window->focused == element && i >= code->selection[0].line &&
                i < code->selection[1].line) {
                UIDrawBlock(painter,
                            UI_RECT_4PD(x, y, code->font->glyphWidth, code->font->glyphHeight),
                            selection.colorBackground);
            }

            if (code->hScroll)
                lineBounds.l += (int64_t)code->hScroll->position;
            painter->clip = oldClip;

            UICodeDecorateLine m;
            m.x = x, m.y = y, m.bounds = lineBounds, m.index = i + 1, m.painter = painter;
            UIElementMessage(element, UI_MSG_CODE_DECORATE_LINE, 0, &m);

            lineBounds.t += lineHeight;
        }

        UIFontActivate(previousFont);
    } else if (message == UI_MSG_SCROLLED) {
        code->moveScrollToFocusNextLayout = false;
        UIElementRefresh(element);
    } else if (message == UI_MSG_MOUSE_WHEEL) {
        return UIElementMessage(&code->vScroll->e, message, di, dp);
    } else if (message == UI_MSG_GET_CURSOR) {
        if (UICodeHitTest(code, element->window->cursorX, element->window->cursorY) < 0) {
            return UI_CURSOR_FLIPPED_ARROW;
        }

        if (element->flags & UI_CODE_SELECTABLE) {
            return UI_CURSOR_TEXT;
        }
    } else if (message == UI_MSG_LEFT_UP) {
        UIElementAnimate(element, true);
    } else if (message == UI_MSG_LEFT_DOWN && code->lineCount) {
        int hitTest = UICodeHitTest(code, element->window->cursorX, element->window->cursorY);
        code->leftDownInMargin = hitTest < 0;

        if (hitTest > 0 && (element->flags & UI_CODE_SELECTABLE)) {
            UICodePositionToByte(code, element->window->cursorX, element->window->cursorY,
                                 &code->selection[2].line, &code->selection[2].offset);
            _UICodeMessage(element, UI_MSG_MOUSE_DRAG, di, dp);
            UIElementFocus(element);
            UIElementAnimate(element, false);
            code->lastAnimateTime = UI_CLOCK();
        }
    } else if (message == UI_MSG_ANIMATE) {
        if (element->window->pressed == element && element->window->pressedButton == 1 &&
            code->lineCount && !code->leftDownInMargin) {
            UI_CLOCK_T previous     = code->lastAnimateTime;
            UI_CLOCK_T current      = UI_CLOCK();
            UI_CLOCK_T deltaTicks   = current - previous;
            double     deltaSeconds = (double)deltaTicks / UI_CLOCKS_PER_SECOND;
            if (deltaSeconds > 0.1)
                deltaSeconds = 0.1;
            int delta = deltaSeconds * 800;
            if (!delta) {
                return 0;
            }
            code->lastAnimateTime = current;

            UIFont *previousFont = UIFontActivate(code->font);

            if (element->window->cursorX <
                element->bounds.l + ((element->flags & UI_CODE_NO_MARGIN)
                                         ? UI_SIZE_CODE_MARGIN_GAP
                                         : (UI_SIZE_CODE_MARGIN + UI_SIZE_CODE_MARGIN_GAP * 2))) {
                code->hScroll->position -= delta;
            } else if (element->window->cursorX >=
                       code->vScroll->e.bounds.l - UI_SIZE_CODE_MARGIN_GAP) {
                code->hScroll->position += delta;
            }

            if (element->window->cursorY < element->bounds.t + UI_SIZE_CODE_MARGIN_GAP) {
                code->vScroll->position -= delta;
            } else if (element->window->cursorY >=
                       code->hScroll->e.bounds.t - UI_SIZE_CODE_MARGIN_GAP) {
                code->vScroll->position += delta;
            }

            code->moveScrollToFocusNextLayout = false;
            UIFontActivate(previousFont);
            _UICodeMessage(element, UI_MSG_MOUSE_DRAG, di, dp);
            UIElementRefresh(element);
        }
    } else if (message == UI_MSG_MOUSE_DRAG && element->window->pressedButton == 1 &&
               code->lineCount && !code->leftDownInMargin) {
        // TODO Double-click and triple-click dragging for word and line granularity respectively.
        UICodePositionToByte(code, element->window->cursorX, element->window->cursorY,
                             &code->selection[3].line, &code->selection[3].offset);
        _UICodeUpdateSelection(code);
        code->moveScrollToFocusNextLayout = code->moveScrollToCaretNextLayout = false;
        code->useVerticalMotionColumn                                         = false;
    } else if (message == UI_MSG_KEY_TYPED && code->lineCount) {
        UIKeyTyped *m = (UIKeyTyped *)dp;

        if ((m->code == UI_KEYCODE_LETTER('C') || m->code == UI_KEYCODE_LETTER('X') ||
             m->code == UI_KEYCODE_INSERT) &&
            element->window->ctrl && !element->window->alt && !element->window->shift) {
            _UICodeCopyText(code);
        } else if ((m->code == UI_KEYCODE_UP || m->code == UI_KEYCODE_DOWN ||
                    m->code == UI_KEYCODE_PAGE_UP || m->code == UI_KEYCODE_PAGE_DOWN) &&
                   !element->window->ctrl && !element->window->alt) {
            UIFont *previousFont = UIFontActivate(code->font);
            int     lineHeight   = UIMeasureStringHeight();

            if (element->window->shift) {
                if (m->code == UI_KEYCODE_UP) {
                    if (code->selection[3].line - 1 >= 0) {
                        _UICodeSetVerticalMotionColumn(code, false);
                        code->selection[3].line--;
                        _UICodeSetVerticalMotionColumn(code, true);
                    }
                } else if (m->code == UI_KEYCODE_DOWN) {
                    if (code->selection[3].line + 1 < code->lineCount) {
                        _UICodeSetVerticalMotionColumn(code, false);
                        code->selection[3].line++;
                        _UICodeSetVerticalMotionColumn(code, true);
                    }
                } else if (m->code == UI_KEYCODE_PAGE_UP || m->code == UI_KEYCODE_PAGE_DOWN) {
                    _UICodeSetVerticalMotionColumn(code, false);
                    int pageHeight =
                        (element->bounds.t - code->hScroll->e.bounds.t) / lineHeight * 4 / 5;
                    code->selection[3].line +=
                        m->code == UI_KEYCODE_PAGE_UP ? pageHeight : -pageHeight;
                    if (code->selection[3].line < 0)
                        code->selection[3].line = 0;
                    if (code->selection[3].line >= code->lineCount)
                        code->selection[3].line = code->lineCount - 1;
                    _UICodeSetVerticalMotionColumn(code, true);
                }

                _UICodeUpdateSelection(code);
            } else {
                code->moveScrollToFocusNextLayout = false;
                _UI_KEY_INPUT_VSCROLL(code, lineHeight,
                                      ((float)(element->bounds.t - code->hScroll->e.bounds.t) * 4 /
                                       5) /* leave a few lines for context */);
            }

            UIFontActivate(previousFont);
        } else if ((m->code == UI_KEYCODE_HOME || m->code == UI_KEYCODE_END) &&
                   !element->window->alt) {
            if (element->window->shift) {
                if (m->code == UI_KEYCODE_HOME) {
                    if (element->window->ctrl)
                        code->selection[3].line = 0;
                    code->selection[3].offset     = 0;
                    code->useVerticalMotionColumn = false;
                } else {
                    if (element->window->ctrl)
                        code->selection[3].line = code->lineCount - 1;
                    code->selection[3].offset     = code->lines[code->selection[3].line].bytes;
                    code->useVerticalMotionColumn = false;
                }

                _UICodeUpdateSelection(code);
            } else {
                code->vScroll->position = m->code == UI_KEYCODE_HOME ? 0 : code->vScroll->maximum;
                code->moveScrollToFocusNextLayout = false;
                UIElementRefresh(&code->e);
            }
        } else if ((m->code == UI_KEYCODE_LEFT || m->code == UI_KEYCODE_RIGHT) &&
                   !element->window->alt) {
            if (element->window->shift) {
                UICodeMoveCaret(code, m->code == UI_KEYCODE_LEFT, element->window->ctrl);
            } else if (!element->window->ctrl) {
                code->hScroll->position += m->code == UI_KEYCODE_LEFT ? -ui.activeFont->glyphWidth
                                                                      : ui.activeFont->glyphWidth;
                UIElementRefresh(&code->e);
            } else {
                return 0;
            }
        } else {
            return 0;
        }

        return 1;
    } else if (message == UI_MSG_RIGHT_DOWN) {
        int hitTest = UICodeHitTest(code, element->window->cursorX, element->window->cursorY);

        if (hitTest > 0 && (element->flags & UI_CODE_SELECTABLE)) {
            UIElementFocus(element);
            UIMenu *menu = UIMenuCreate(&element->window->e, UI_MENU_NO_SCROLL);
            UIMenuAddItem(menu,
                          (code->selection[0].line == code->selection[1].line &&
                           code->selection[0].offset == code->selection[1].offset)
                              ? UI_ELEMENT_DISABLED
                              : 0,
                          "Copy", -1, _UICodeCopyText, code);
            UIMenuShow(menu);
        }
    } else if (message == UI_MSG_UPDATE) {
        UIElementRepaint(element, NULL);
    } else if (message == UI_MSG_DEALLOCATE) {
        UI_FREE(code->content);
        UI_FREE(code->lines);
    }

    return 0;
}


//


int _UICodeByteToColumn(UICode *code, int line, int byte)
{
    return _UIByteToColumn(&code->content[code->lines[line].offset], byte, code->lines[line].bytes,
                           code->tabSize);
}

int _UICodeColumnToByte(UICode *code, int line, int column)
{
    return _UIColumnToByte(&code->content[code->lines[line].offset], column,
                           code->lines[line].bytes, code->tabSize);
}

void UICodePositionToByte(UICode *code, int x, int y, int *line, int *byte)
{
    UIFont *previousFont = UIFontActivate(code->font);
    int     lineHeight   = UIMeasureStringHeight();
    *line                = (y - code->e.bounds.t + code->vScroll->position) / lineHeight;
    if (*line < 0)
        *line = 0;
    else if (*line >= code->lineCount)
        *line = code->lineCount - 1;
    int column =
        (x - code->e.bounds.l + code->hScroll->position + ((float)ui.activeFont->glyphWidth / 2)) /
        ui.activeFont->glyphWidth;
    if (~code->e.flags & UI_CODE_NO_MARGIN)
        column -= (UI_SIZE_CODE_MARGIN + UI_SIZE_CODE_MARGIN_GAP) / ui.activeFont->glyphWidth;
    UIFontActivate(previousFont);
    *byte = _UICodeColumnToByte(code, *line, column);
}


int UICodeHitTest(UICode *code, int x, int y)
{
    x -= code->e.bounds.l;

    if (x < 0 || x >= code->vScroll->e.bounds.l) {
        return 0;
    }

    y -= code->e.bounds.t - code->vScroll->position;

    UIFont *previousFont = UIFontActivate(code->font);
    int     lineHeight   = UIMeasureStringHeight();
    bool    inMargin     = x < UI_SIZE_CODE_MARGIN + UI_SIZE_CODE_MARGIN_GAP / 2 &&
                    (~code->e.flags & UI_CODE_NO_MARGIN);
    UIFontActivate(previousFont);

    if (y < 0 || y >= lineHeight * code->lineCount) {
        return 0;
    }

    int line = y / lineHeight + 1;
    return inMargin ? -line : line;
}


int UIDrawStringHighlighted(UIPainter *painter, UIRectangle lineBounds, const char *string,
                            ptrdiff_t bytes, int tabSize, UIStringSelection *selection)
{
    if (bytes == -1)
        bytes = _UIStringLength(string);
    if (bytes > 10000)
        bytes = 10000;

    typedef enum _UICodeTokenType {
        UI_CODE_TOKEN_TYPE_DEFAULT,
        UI_CODE_TOKEN_TYPE_COMMENT,
        UI_CODE_TOKEN_TYPE_STRING,
        UI_CODE_TOKEN_TYPE_NUMBER,
        UI_CODE_TOKEN_TYPE_OPERATOR,
        UI_CODE_TOKEN_TYPE_PREPROCESSOR,
    } _UICodeTokenType;

    uint32_t colors[] = {
        ui.theme.codeDefault, ui.theme.codeComment,  ui.theme.codeString,
        ui.theme.codeNumber,  ui.theme.codeOperator, ui.theme.codePreprocessor,
    };

    int              lineHeight = UIMeasureStringHeight();
    int              x          = lineBounds.l;
    int              y          = (lineBounds.t + lineBounds.b - lineHeight) / 2;
    int              ti         = 0;
    _UICodeTokenType tokenType  = UI_CODE_TOKEN_TYPE_DEFAULT;
    bool             inComment = false, inIdentifier = false, inChar = false, startedString = false,
         startedPreprocessor = false;
    uint32_t last            = 0;
    int      j               = 0;

    while (bytes) {
#ifdef UI_UNICODE
        ptrdiff_t bytesConsumed;
        int       c = Utf8GetCodePoint(string, bytes, &bytesConsumed);
        UI_ASSERT(bytesConsumed > 0);
        string += bytesConsumed;
        bytes -= bytesConsumed;
#else
        char c = *string++;
        bytes--;
#endif

        last <<= 8;
        last |= c & 0xFF;

        if (tokenType == UI_CODE_TOKEN_TYPE_PREPROCESSOR) {
            if (bytes && c == '/' && (*string == '/' || *string == '*')) {
                tokenType = UI_CODE_TOKEN_TYPE_DEFAULT;
            }
        } else if (tokenType == UI_CODE_TOKEN_TYPE_OPERATOR) {
            tokenType = UI_CODE_TOKEN_TYPE_DEFAULT;
        } else if (tokenType == UI_CODE_TOKEN_TYPE_COMMENT) {
            if ((last & 0xFF0000) == ('*' << 16) && (last & 0xFF00) == ('/' << 8) && inComment) {
                tokenType = startedPreprocessor ? UI_CODE_TOKEN_TYPE_PREPROCESSOR
                                                : UI_CODE_TOKEN_TYPE_DEFAULT;
                inComment = false;
            }
        } else if (tokenType == UI_CODE_TOKEN_TYPE_NUMBER) {
            if (!_UICharIsAlpha(c) && !_UICharIsDigit(c)) {
                tokenType = UI_CODE_TOKEN_TYPE_DEFAULT;
            }
        } else if (tokenType == UI_CODE_TOKEN_TYPE_STRING) {
            if (!startedString) {
                if (!inChar && ((last >> 8) & 0xFF) == '"' && ((last >> 16) & 0xFF) != '\\') {
                    tokenType = UI_CODE_TOKEN_TYPE_DEFAULT;
                } else if (inChar && ((last >> 8) & 0xFF) == '\'' &&
                           ((last >> 16) & 0xFF) != '\\') {
                    tokenType = UI_CODE_TOKEN_TYPE_DEFAULT;
                }
            }

            startedString = false;
        }

        if (tokenType == UI_CODE_TOKEN_TYPE_DEFAULT) {
            if (c == '#') {
                tokenType           = UI_CODE_TOKEN_TYPE_PREPROCESSOR;
                startedPreprocessor = true;
            } else if (bytes && c == '/' && *string == '/') {
                tokenType = UI_CODE_TOKEN_TYPE_COMMENT;
            } else if (bytes && c == '/' && *string == '*') {
                tokenType = UI_CODE_TOKEN_TYPE_COMMENT, inComment = true;
            } else if (c == '"') {
                tokenType     = UI_CODE_TOKEN_TYPE_STRING;
                inChar        = false;
                startedString = true;
            } else if (c == '\'') {
                tokenType     = UI_CODE_TOKEN_TYPE_STRING;
                inChar        = true;
                startedString = true;
            } else if (_UICharIsDigit(c) && !inIdentifier) {
                tokenType = UI_CODE_TOKEN_TYPE_NUMBER;
            } else if (!_UICharIsAlpha(c) && !_UICharIsDigit(c)) {
                tokenType    = UI_CODE_TOKEN_TYPE_OPERATOR;
                inIdentifier = false;
            } else {
                inIdentifier = true;
            }
        }

        int oldX = x;

        if (c == '\t') {
            x += ui.activeFont->glyphWidth, ti++;
            while (ti % tabSize)
                x += ui.activeFont->glyphWidth, ti++, j++;
        } else {
            UIDrawGlyph(painter, x, y, c, colors[tokenType]);
            x += ui.activeFont->glyphWidth, ti++;
        }

        if (selection && j >= selection->carets[0] && j < selection->carets[1]) {
            UIDrawBlock(painter, UI_RECT_4(oldX, x, y, y + lineHeight), selection->colorBackground);
            if (c != '\t')
                UIDrawGlyph(painter, oldX, y, c, selection->colorText);
        }

        if (selection && selection->carets[0] == j) {
            UIDrawInvert(painter, UI_RECT_4(oldX, oldX + 1, y, y + lineHeight));
        }

        j++;
    }

    if (selection && selection->carets[0] == j) {
        UIDrawInvert(painter, UI_RECT_4(x, x + 1, y, y + lineHeight));
    }

    return x;
}

void _UICodeUpdateSelection(UICode *code)
{
    bool swap = code->selection[3].line < code->selection[2].line ||
                (code->selection[3].line == code->selection[2].line &&
                 code->selection[3].offset < code->selection[2].offset);
    code->selection[1 - swap]         = code->selection[3];
    code->selection[0 + swap]         = code->selection[2];
    code->moveScrollToCaretNextLayout = true;
    UIElementRefresh(&code->e);
}

void _UICodeSetVerticalMotionColumn(UICode *code, bool restore)
{
    if (restore) {
        code->selection[3].offset =
            _UICodeColumnToByte(code, code->selection[3].line, code->verticalMotionColumn);
    } else if (!code->useVerticalMotionColumn) {
        code->useVerticalMotionColumn = true;
        code->verticalMotionColumn =
            _UICodeByteToColumn(code, code->selection[3].line, code->selection[3].offset);
    }
}

void _UICodeCopyText(void *cp)
{
    UICode *code = (UICode *)cp;

    int from = code->lines[code->selection[0].line].offset + code->selection[0].offset;
    int to   = code->lines[code->selection[1].line].offset + code->selection[1].offset;

    if (from != to) {
        char *pasteText = (char *)UI_CALLOC(to - from + 2);
        for (int i = from; i < to; i++)
            pasteText[i - from] = code->content[i];
        _UIClipboardWriteText(code->e.window, pasteText);
    }
}

void UICodeMoveCaret(UICode *code, bool backward, bool word)
{
    while (true) {
        if (backward) {
            if (code->selection[3].offset - 1 < 0) {
                if (code->selection[3].line > 0) {
                    code->selection[3].line--;
                    code->selection[3].offset = code->lines[code->selection[3].line].bytes;
                } else
                    break;
            } else
                _UI_MOVE_CARET_BACKWARD(code->selection[3].offset, code->content,
                                        code->lines[code->selection[3].line].offset +
                                            code->selection[3].offset,
                                        code->lines[code->selection[3].line].offset);
        } else {
            if (code->selection[3].offset + 1 > code->lines[code->selection[3].line].bytes) {
                if (code->selection[3].line + 1 < code->lineCount) {
                    code->selection[3].line++;
                    code->selection[3].offset = 0;
                } else
                    break;
            } else
                _UI_MOVE_CARET_FORWARD(code->selection[3].offset, code->content, code->contentBytes,
                                       code->lines[code->selection[3].line].offset +
                                           code->selection[3].offset);
        }

        if (!word)
            break;

        if (code->selection[3].offset != 0 &&
            code->selection[3].offset != code->lines[code->selection[3].line].bytes) {
            _UI_MOVE_CARET_BY_WORD(code->content, code->contentBytes,
                                   code->lines[code->selection[3].line].offset +
                                       code->selection[3].offset);
        }
    }

    code->useVerticalMotionColumn = false;
    _UICodeUpdateSelection(code);
}


void UICodeFocusLine(UICode *code, int index)
{
    code->focused                     = index - 1;
    code->moveScrollToFocusNextLayout = true;
    UIElementRefresh(&code->e);
}


void UICodeInsertContent(UICode *code, const char *content, ptrdiff_t byteCount, bool replace)
{
    code->useVerticalMotionColumn = false;

    UIFont *previousFont = UIFontActivate(code->font);

    if (byteCount == -1) {
        byteCount = _UIStringLength(content);
    }

    if (byteCount > 1000000000) {
        byteCount = 1000000000;
    }

    if (replace) {
        UI_FREE(code->content);
        UI_FREE(code->lines);
        code->content           = NULL;
        code->lines             = NULL;
        code->contentBytes      = 0;
        code->lineCount         = 0;
        code->columns           = 0;
        code->selection[0].line = code->selection[1].line = 0;
        code->selection[0].offset = code->selection[1].offset = 0;
    }

    code->content = (char *)UI_REALLOC(code->content, code->contentBytes + byteCount);

    if (!byteCount) {
        return;
    }

    int lineCount = content[byteCount - 1] != '\n';

    for (int i = 0; i < byteCount; i++) {
        code->content[i + code->contentBytes] = content[i];

        if (content[i] == '\n') {
            lineCount++;
        }
    }

    code->lines =
        (UICodeLine *)UI_REALLOC(code->lines, sizeof(UICodeLine) * (code->lineCount + lineCount));
    int offset = 0, lineIndex = 0;

    for (intptr_t i = 0; i <= byteCount && lineIndex < lineCount; i++) {
        if (content[i] == '\n' || i == byteCount) {
            UICodeLine line = {0};
            line.offset     = offset + code->contentBytes;
            line.bytes      = i - offset;
            if (line.bytes > code->columns)
                code->columns = line.bytes;
            code->lines[code->lineCount + lineIndex] = line;
            lineIndex++;
            offset = i + 1;
        }
    }

    code->lineCount += lineCount;
    code->contentBytes += byteCount;

    if (!replace) {
        code->vScroll->position = code->lineCount * UIMeasureStringHeight();
    }

    UIFontActivate(previousFont);
    UIElementRepaint(&code->e, NULL);
}

UICode *UICodeCreate(UIElement *parent, uint32_t flags)
{
    UICode *code = (UICode *)UIElementCreate(sizeof(UICode), parent, flags, _UICodeMessage, "Code");
    code->font   = ui.activeFont;
    code->vScroll = UIScrollBarCreate(&code->e, 0);
    code->hScroll = UIScrollBarCreate(&code->e, UI_SCROLL_BAR_HORIZONTAL);
    code->focused = -1;
    code->tabSize = 4;
    return code;
}


UICode *UICodeCreateFromFile(UIElement *parent, uint32_t flags, const char *file_name)
{
    UICode *code = UICodeCreate(parent, flags);

    if (file_name) {
        FILE *f = fopen(file_name, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            const size_t sz     = ftell(f);
            char        *buffer = (char *)UI_CALLOC(sz + 1);
            fseek(f, 0, SEEK_SET);
            size_t size = fread(buffer, 1, sz, f);
            fclose(f);
            UICodeInsertContent(code, buffer, size, true);
            UICodeFocusLine(code, 0);
        }
    }

    return code;
}
