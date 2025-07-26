#include "common.h"
#include "mini_uart.h"
#include "gpio.h"
#include "utils.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"

void putc(void *p , char c){
    if(c == '\n'){
        uart_send('\r');
    }
    
    uart_send(c);
}

u32  get_el();

void kernel_main() {
    uart_init();
    init_printf(0,putc);
    gpio_init(GFOutput);
    // uart_send_string("Rasperry PI Bare Metal OS Initializing...\n");
    // gpio_debug();
    printf("Rasperry PI Bare Metal OS Initializing...\n");
    
    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable(); 
    timer_init();
    printf("Waiting for 200ms");
    timer_sleep(200);

#if RPI_VERSION == 3
    printf("\tBoard: Raspberry PI 3\n");
    
#endif

#if RPI_VERSION == 4
    printf("\tBoard: Raspberry PI 4\n");
#endif

    printf ("\nException Level: %d \n",get_el());
    while(1) {
    }
}
