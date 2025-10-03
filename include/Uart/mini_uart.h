#pragma once

#include "peripherals/aux.h"

#define TXD 14
#define RXD 15

void uart_init();
char uart_recv();
void uart_send(char c);
void uart_send_string(char *str);

bool uart_is_readable();
bool uart_is_writable();
void uart_flush();