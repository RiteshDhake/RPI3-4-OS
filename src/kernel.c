#include "common.h"
#include "Uart/UartDemo.hpp"
#include "Gpio/gpio.h"
#include "utils.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"
#include "mailbox.h"
#include "Graphics/compositor.h"
#include "Uart/mini_uart.h"
#include "mem.h"
#include "heap_allocator.h"

extern void run_graphics_demo();
extern void run_uart_demo();

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
    gpio_init_all(GFOutput);
    // uart_send_string("Rasperry PI Bare Metal OS Initializing...\n");
    // gpio_debug();
    printf("Rasperry PI Bare Metal OS Initializing...\n");
    printf ("\nException Level: %d \n",get_el()); 
    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable(); 
    timer_init(); 
    printf("Waiting for 200ms\n");
    timer_sleep(200);
#if RPI_VERSION == 3
    printf("\tBoard: Raspberry PI 3\n");
    
#endif

#if RPI_VERSION == 4
    printf("\tBoard: Raspberry PI 4\n");
#endif
    printf("MAILBOX:\n");

    printf("CORE CLOCK: %d\n", mailbox_clock_rate(CT_CORE));
    printf("EMMC CLOCK: %d\n", mailbox_clock_rate(CT_EMMC));
    printf("UART CLOCK: %d\n", mailbox_clock_rate(CT_UART));
    printf("ARM  CLOCK: %d\n", mailbox_clock_rate(CT_ARM));

    u32 max_temp = 0;

    mailbox_generic_command(RPI_FIRMWARE_GET_MAX_TEMPERATURE, 0, &max_temp);

    //Do video...
    // void *p1 = get_free_pages(10);
    // void *p2 = get_free_pages(4);
    // void *p3 = allocate_memory(20 * 4096 + 1);

    // free_memory(p1);
    // free_memory(p2);
    // free_memory(p3);
    // timer_sleep(500);

    page_stress_test();
    heap_stress_test();

    demo_usage();

    
    while(1) {
        u32 cur_temp = 0;

        mailbox_generic_command(RPI_FIRMWARE_GET_TEMPERATURE, 0, &cur_temp);

        printf("Cur temp: %dC MAX: %dC\n", cur_temp / 1000, max_temp / 1000);

        timer_sleep(1000);
    
    }
}
