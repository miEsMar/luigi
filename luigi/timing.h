#ifndef LUIGI_TIMING_H_
#define LUIGI_TIMING_H_


#ifdef __cplusplus
extern "C" {
#endif


#if defined(UI_LINUX) || defined(UI_COCOA)
# include <time.h>

# define UI_CLOCK             _UIClock
# define UI_CLOCKS_PER_SECOND 1000
# define UI_CLOCK_T           clock_t

static inline UI_CLOCK_T _UIClock(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}
#endif
#ifdef UI_WINDOWS
# include <windows.h>

# define UI_CLOCK             GetTickCount
# define UI_CLOCKS_PER_SECOND (1000)
# define UI_CLOCK_T           DWORD
#endif
#if defined(UI_ESSENCE)
# include <essence.h>

# define UI_CLOCK             EsTimeStampMs
# define UI_CLOCKS_PER_SECOND 1000
# define UI_CLOCK_T           uint64_t
#endif


#ifdef __cplusplus
}
#endif


#endif // LUIGI_TIMING_H_
