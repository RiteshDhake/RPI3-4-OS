#ifndef ASSERT_H
#define ASSERT_H

#include"libcpp/types.h"

// extern "C" if you want to call from C++
#ifdef __cplusplus
extern "C" {
#endif

// Low-level panic function with UART print + halt 
void panic(const char* msg, const char* file, int line);

#ifdef __cplusplus
}
#endif

// If NDEBUG is defined, assertions disappear
#ifdef NDEBUG
    #define assert(expr) ((void)0)
#else
    #define assert(expr) \
        ((expr) ? (void)0 : panic("Assertion failed: " #expr, __FILE__, __LINE__))
#endif

#endif // ASSERT_H