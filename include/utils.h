#pragma once
#include "common.h"

void delay(u64 ticks);
void put32(u64 address , u32 val); 
u32 get32(u64 address);
void delay_seconds(u64 seconds);
void debug(char *str);
u32 get_el();

