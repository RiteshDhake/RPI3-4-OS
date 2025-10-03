#include "Gpio/GpioInterop.hpp"
#include "libcpp/assert.h"
#include "libcpp/types.h"
#include "Gpio/GpioAPI.hpp"

using namespace GPIO;

void Pin::init(){
    gpio_init(number,GFOutput);
    enabled = true;
}

void Pin::set(){
    gpio_set(number);
}

void Pin::clear(){
    gpio_clear(number);
}

u8 Pin::getNumber(){
    return number;
}

GpioFunc Pin::getfunc(){
    return func;
}



