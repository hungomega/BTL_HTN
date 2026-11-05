#ifndef HAL_UART_H
#define HAL_UART_H

#include "stm32f10x.h"

void hal_uart_init(void);
void hal_uart_send_char(char data);
void hal_uart_send_string(const char *str);

#endif