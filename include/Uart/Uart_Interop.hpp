#ifndef Uart_Interop_H
#define Uart_Interop_H


// bridge between CPP and C; Allows CPP to use C functions

extern "C" {
    #include "mini_uart.h"
    #include "printf.h"

//     typedef void (*putcf)(void*, char);

// void init_printf(void* putp, void (*putf)(void*,char));
// void tfp_format(void* putp, putcf putf, char *fmt, va_list va);
// void tfp_printf(char *fmt, ...);
// void tfp_sprintf(char* s, char *fmt, ...);
}

#endif
