

#ifdef UI_IMPLEMENTATION

/////////////////////////////////////////
// Automation for tests.
/////////////////////////////////////////

# ifdef UI_AUTOMATION_TESTS

int UIAutomationRunTests();

void UIAutomationProcessMessage()
{
    int result;
    _UIMessageLoopSingle(&result);
}

void UIAutomationKeyboardTypeSingle(intptr_t code, bool ctrl, bool shift, bool alt)
{
    UIWindow  *window = ui.windows; // TODO Get the focused window.
    UIKeyTyped m      = {0};
    m.code            = code;
    window->ctrl      = ctrl;
    window->alt       = alt;
    window->shift     = shift;
    _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
    window->ctrl  = false;
    window->alt   = false;
    window->shift = false;
}

void UIAutomationKeyboardType(const char *string)
{
    UIWindow *window = ui.windows; // TODO Get the focused window.

    UIKeyTyped m = {0};
    char       c[2];
    m.text      = c;
    m.textBytes = 1;
    c[1]        = 0;

    for (int i = 0; string[i]; i++) {
        window->ctrl  = false;
        window->alt   = false;
        window->shift = (c[0] >= 'A' && c[0] <= 'Z');
        c[0]          = string[i];
        m.code        = (c[0] >= 'A' && c[0] <= 'Z')   ? UI_KEYCODE_LETTER(c[0])
                        : c[0] == '\n'                 ? UI_KEYCODE_ENTER
                        : c[0] == '\t'                 ? UI_KEYCODE_TAB
                        : c[0] == ' '                  ? UI_KEYCODE_SPACE
                        : (c[0] >= '0' && c[0] <= '9') ? UI_KEYCODE_DIGIT(c[0])
                                                       : 0;
        _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
    }

    window->ctrl  = false;
    window->alt   = false;
    window->shift = false;
}

bool UIAutomationCheckCodeLineMatches(UICode *code, int lineIndex, const char *input)
{
    if (lineIndex < 1 || lineIndex > code->lineCount)
        return false;
    int bytes = 0;
    for (int i = 0; input[i]; i++)
        bytes++;
    if (bytes != code->lines[lineIndex - 1].bytes)
        return false;
    for (int i = 0; input[i]; i++)
        if (code->content[code->lines[lineIndex - 1].offset + i] != input[i])
            return false;
    return true;
}

bool UIAutomationCheckTableItemMatches(UITable *table, int row, int column, const char *input)
{
    int bytes = 0;
    for (int i = 0; input[i]; i++)
        bytes++;
    if (row < 0 || row >= table->itemCount)
        return false;
    if (column < 0 || column >= table->columnCount)
        return false;
    char          *buffer = (char *)UI_MALLOC(bytes + 1);
    UITableGetItem m      = {0};
    m.buffer              = buffer;
    m.bufferBytes         = bytes + 1;
    m.column              = column;
    m.index               = row;
    int length            = UIElementMessage(&table->e, UI_MSG_TABLE_GET_ITEM, 0, &m);
    if (length != bytes)
        return false;
    for (int i = 0; input[i]; i++)
        if (buffer[i] != input[i])
            return false;
    return true;
}

# endif

#endif // UI_IMPLEMENTATION
