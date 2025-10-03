#include"Uart/UartAPI.hpp"
#include"Uart/Uart_Interop.hpp"
#include "Uart/UartDemo.hpp"
using namespace Uart;

void run_uart_demo(){
	Uart::MiniUart uart;

	uart.init();

	uart.printf("Hello\n");
}


