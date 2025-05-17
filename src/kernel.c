#include "common.h"
#include "mini_uart.h"
#include "gpio.h"
#include "utils.h"

void kernel_main() {
    uart_init();
    gpio_init(GFOutput);
    // uart_send_string("Rasperry PI Bare Metal OS Initializing...\n");
    // gpio_debug();
    debug("Rasperry PI Bare Metal OS Initializing...\n");

    

#if RPI_VERSION == 3
    uart_send_string("\tBoard: Raspberry PI 3\n");
    
#endif

#if RPI_VERSION == 4
    uart_send_string("\tBoard: Raspberry PI 4\n");
#endif

    // uart_send_string("\n\nDone\n");
    debug("\n\nDone\n");
    
    while(1) {
        uart_send(uart_recv());
        
    }
}