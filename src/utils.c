#include "common.h"
#include "utils.h"
#include "gpio.h"
#include "mini_uart.h"


void debug(char *str){
    uart_send_string(str);
    gpio_debug();

    delay_seconds(2);
}

