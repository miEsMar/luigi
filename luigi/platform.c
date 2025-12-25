#include "platform.h"

#ifdef UI_LINUX
# include "platforms/linux.c"
#endif // UI_LINUX
#ifdef UI_WINDOWS
# include "platforms/windows.c"
#endif // UI_WINDOWS
#ifdef UI_ESSENCE
# include "platforms/essence.c"
#endif // UI_ESSENCE
#ifdef UI_COCOA
# include "platforms/cocoa.c"
#endif // UI_COCOA
