#include "libcpp/assert.h"
#include "printf.h"   // your UART/console printf
#include "timer.h"    // if you want to halt timer
#include <stddef.h>

void panic(const char* msg, const char* file, int line) {
    printf("\nKERNEL PANIC!\n");
    printf("%s at %s:%d\n", msg, file, line);

    // Halt the system
    while (1) {
        __asm__ volatile("wfe"); // Wait for event (low-power spin)
    }
}
