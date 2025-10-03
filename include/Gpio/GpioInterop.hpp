#ifndef GPIO_INTEROP_H
#define GPIO_INTEROP_H


// bridge between CPP and C; Allows CPP to use C functions

extern "C" {
    #include "gpio.h"
}

#endif