#include "Uart/UartAPI.hpp"
#include "Uart/Uart_Interop.hpp"
#include "libcpp/assert.h"
#include "libcpp/types.h"
// #include "printf.h"
#include <stdarg.h>

#define THROW_ERROR(msg) panic(msg, __FILE__, __LINE__)





namespace Uart {   // wrap everything in the Uart namespace

bool MiniUart::init() {
    if (initialized) return true;
    
    uart_init();
    initialized = true;
    
    // Initialize printf to use this UART
    init_printf(this, uart_putc_callback);
    
    return true;
}

void MiniUart::send(char c) {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    uart_send(c);
}

void MiniUart::send_str(const char* str) {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    if (!str) return;
    
    uart_send_string(const_cast<char*>(str));
}

char MiniUart::recv() {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    return uart_recv();
}

bool MiniUart::isReadable() const {
    if (!initialized) return false;
    return uart_is_readable();
}

bool MiniUart::isWritable() const {
    if (!initialized) return false;
    return uart_is_writable();
}

void MiniUart::flush() {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    uart_flush();
}

void MiniUart::println(const char* str) {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    if (str) {
        send_str(str);
    }
    send('\n');
}

// Static callback for printf integration
void MiniUart::uart_putc_callback(void* p, char c) {
    MiniUart* uart = static_cast<MiniUart*>(p);
    if (uart && uart->initialized) {
        uart->send(c);
    }
}

// Printf implementation
void MiniUart::printf(const char* format, ...) {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    
    va_list args;
    va_start(args, format);
    tfp_format(this, uart_putc_callback, (char*)format, args);
    va_end(args);
}

void MiniUart::vprintf(const char* format, va_list args) {
    if (!initialized) {
        THROW_ERROR("MiniUart not initialized!");
    }
    
    // Use tinyprintf vprintf variant
     tfp_format(this, uart_putc_callback, (char*)format, args);
}

bool MiniUart::tryRecv(char& c) {
    if (!initialized) return false;
    
    if (isReadable()) {
        c = recv();
        return true;
    }
    return false;
}

size_t MiniUart::available() const {
    return isReadable() ? 1 : 0;
}

// Singleton
MiniUart& getUart() {
    static MiniUart instance;
    return instance;
}

// Global printf initialization
void uart_printf_init() {
    auto& uart = getUart();
    uart.init();
    // printf now automatically uses UART
}

} // namespace Uart


