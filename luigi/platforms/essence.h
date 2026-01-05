#ifndef LUIGI_PLATFORM_ESSENCE_H_
#define LUIGI_PLATFORM_ESSENCE_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_ESSENCE
typedef struct Luigi_Platform_Essence {
    EsInstance *instance;
    void       *menuData[256]; // HACK This limits the number of menu items to 128.
    uintptr_t   menuIndex;
} Luigi_Platform_Essence;


typedef struct Luigi_PlatformWindow_Essence {
    EsWindow  *window;
    EsElement *canvas;
    int        cursor;
} Luigi_PlatformWindow_Essence;
#endif


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_ESSENCE_H_
