#ifndef GPIOAPI_HPP
#define GPIOAPI_HPP

#include "libcpp/types.h"
#include "gpio.h"

namespace GPIO{
    class Pin{
        private:
            u8 number;
            GpioFunc func;
            bool enabled;
        
        public:
            Pin(u8 pinNumber, GpioFunc function = GFOutput)
                : number(pinNumber),func(function),enabled(false){}

            void init();
            void set();
            void clear();
            u8 getNumber();
            GpioFunc getfunc();
    };
}

#endif