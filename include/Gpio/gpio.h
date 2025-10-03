#pragma once 

#include"peripherals/gpio.h"

typedef enum _GpioFunc {
    GFInput = 0,
    GFOutput = 1,
    GFAlt0 = 4,
    GFAlt1 = 5,
    GFAlt2 = 6,
    GFAlt3 = 7,
    GFAlt4 =3,
    GFAlt5 =2
} GpioFunc;

void gpio_pin_set_func(u8 pinNumber , GpioFunc func);

void gpio_pin_enable(u8 pinNumber);

void gpio_init(u8 pinNumber, GpioFunc func);

void gpio_set(u8 pinNumber);

void gpio_clear(u8 pinNumber);

void gpio_debug();

void gpio_init_all( GpioFunc func);