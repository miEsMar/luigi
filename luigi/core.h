#ifndef LUIGI_CORE_H_
#define LUIGI_CORE_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>


#ifdef UI_SSE2
# include <xmmintrin.h>
#endif

#ifdef UI_AVX512
# include <immintrin.h>
#endif

#ifdef UI_WINDOWS
# include <windows.h>

# define UI_ASSERT(x)                                                                              \
     do {                                                                                          \
         if (!(x)) {                                                                               \
             ui.assertionFailure = true;                                                           \
             MessageBox(0, "Assertion failure on line " _UI_TO_STRING_2(__LINE__), 0, 0);          \
             ExitProcess(1);                                                                       \
         }                                                                                         \
     } while (0)
# define UI_CALLOC(x)         HeapAlloc(ui.heap, HEAP_ZERO_MEMORY, (x))
# define UI_FREE(x)           HeapFree(ui.heap, 0, (x))
# define UI_MALLOC(x)         HeapAlloc(ui.heap, 0, (x))
# define UI_REALLOC           _UIHeapReAlloc
# define UI_CLOCK             GetTickCount
# define UI_CLOCKS_PER_SECOND (1000)
# define UI_CLOCK_T           DWORD
# define UI_MEMMOVE           _UIMemmove
#endif


#endif // LUIGI_CORE_H_
