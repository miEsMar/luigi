#ifndef LUIGI_INPUT_H_
#define LUIGI_INPUT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef struct UIKeyTyped {
    char    *text;
    int      textBytes;
    intptr_t code;
} UIKeyTyped;


#ifdef __cplusplus
}
#endif


#endif // LUIGI_INPUT_H_
