#include "gpio.h"
#include "utils.h"

#define GPIO_PINS {1,2,3,4,5,6}  // List of GPIO pin numbers


void gpio_pin_set_func(u8 pinNumber , GpioFunc func){
    u8 bitstart = (pinNumber *3)%30;
    u8 reg = pinNumber/10;
 
    u32 selector = REGS_GPIO->func_select[reg];
    
    selector &= ~(7 << bitstart);
    selector|=(func << bitstart);
    
    REGS_GPIO->func_select[reg] = selector;

}

void gpio_pin_enable(u8 pinNumber){
    REGS_GPIO->pupd_enable = 0;
    delay(150);
    REGS_GPIO->pupd_enable_clocks[pinNumber/32] = 1<<(pinNumber%32);
    delay(150);
    REGS_GPIO->pupd_enable = 0;
    REGS_GPIO->pupd_enable_clocks[pinNumber/32] = 0;
}

void gpio_init(GpioFunc func){

    const int output_pins[] = GPIO_PINS;
    size numPins  =( sizeof(output_pins)/sizeof(output_pins[0]));

    for (int i = 0 ; i < numPins ; i++){
        gpio_pin_set_func(output_pins[i],func);
        gpio_pin_enable(output_pins[i]);
    }
}

void gpio_set(u8 pinNumber){
    REGS_GPIO->output_set.data[pinNumber/32] = (1<<(pinNumber%32));
}

void gpio_clear(u8 pinNumber){
    REGS_GPIO->output_clear.data[pinNumber/32] = (1<<(pinNumber%32));
}

void delay_seconds(u64 seconds){
    u64 expo = 10000000;//(10^7)
    expo = expo * seconds;
    delay(expo);
}

//this a temporary function created to debug 
//, because my laptop has unreliable  USB to TTL(UART communication)support

void gpio_debug(){
    const int output_pins[] = GPIO_PINS;
    size numPins  =(sizeof(output_pins)/sizeof(output_pins[0]));

    for (int i = 0 ; i < numPins ; i++){
        gpio_set(output_pins[i]);
    } 
    delay(5000000);

    for (int i = 0 ; i < numPins ; i++){
        gpio_clear(output_pins[i]);
    } 

}