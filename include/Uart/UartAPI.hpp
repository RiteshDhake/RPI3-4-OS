#pragma once

#include "libcpp/types.h"
#include <stdarg.h>
#include <stddef.h>
#include "printf.h"

namespace Uart{

    class MiniUart{
        private:
            bool initialized = false;
        public:
          MiniUart() = default;
        ~MiniUart() = default;

        // Basic operations
        bool init();
        void send(char c);
        void send_str(const char* str);
        char recv();
        
        // Status checks
        bool isReadable() const;
        bool isWritable() const;
        bool isInitialized() const { return initialized; }
        
        // Enhanced functionality
        void flush();
        void println(const char* str);
        
        // Printf integration - now properly implemented
        void printf(const char* format, ...);
        void vprintf(const char* format, va_list args);
        
        // Non-blocking operations
        bool tryRecv(char& c);
        size_t available() const;

        void uart_printf_init();
        // Static callback for printf integration
        static void uart_putc_callback(void* p, char c);
    };
    MiniUart& getUart();
    
}
