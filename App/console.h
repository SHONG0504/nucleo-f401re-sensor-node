/* A line-oriented command shell on USART2. Bytes arrive by interrupt and are
 * queued; console_poll() runs from the main loop, assembles lines and
 * dispatches commands. Output goes through printf (see __io_putchar in
 * main.c). */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "stm32f4xx_hal.h"

/* Start receiving on huart. Call once after the UART is initialised. */
void console_init(UART_HandleTypeDef *huart);

/* Run pending input: call from the main loop, often. */
void console_poll(void);

/* Called from HAL_UART_RxCpltCallback for the console's UART. */
void console_rx_complete(UART_HandleTypeDef *huart);

#endif
