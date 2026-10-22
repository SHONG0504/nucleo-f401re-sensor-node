#include "console.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "button.h"
#include "main.h"
#include "sampler.h"

#define RX_QUEUE_SIZE 64u   /* bytes queued between interrupt and main loop */
#define LINE_MAX      32u   /* longest command line, terminator excluded */

static UART_HandleTypeDef *s_uart;
static uint8_t s_rx_byte;

/* Interrupt -> main loop queue. Single producer (ISR), single consumer. */
static volatile uint8_t s_queue[RX_QUEUE_SIZE];
static volatile uint32_t s_head; /* ISR writes */
static volatile uint32_t s_tail; /* poll reads */

static char s_line[LINE_MAX + 1];
static uint32_t s_line_len;

static void cmd_help(void)
{
  printf("commands:\r\n");
  printf("  help         this text\r\n");
  printf("  status       uptime, last sample, LED\r\n");
  printf("  led on|off   drive LD2\r\n");
}

static void cmd_status(void)
{
  uint32_t ms = HAL_GetTick();
  printf("uptime: %lu.%03lu s\r\n", (unsigned long)(ms / 1000u), (unsigned long)(ms % 1000u));
  printf("samples: %lu, last %u mV\r\n", (unsigned long)sampler_count(), sampler_last_mv());
  printf("led: %s\r\n", button_led_is_on() ? "on" : "off");
}

static void cmd_led(const char *arg)
{
  if (strcmp(arg, "on") == 0) {
    button_set_led(true);
  } else if (strcmp(arg, "off") == 0) {
    button_set_led(false);
  } else {
    printf("usage: led on|off\r\n");
    return;
  }
  printf("led: %s\r\n", button_led_is_on() ? "on" : "off");
}

static void dispatch(char *line)
{
  /* Split the first word from the rest. */
  char *arg = line;
  while (*arg != '\0' && !isspace((unsigned char)*arg)) {
    arg++;
  }
  if (*arg != '\0') {
    *arg++ = '\0';
    while (isspace((unsigned char)*arg)) {
      arg++;
    }
  }
  if (line[0] == '\0') {
    return;
  }
  if (strcmp(line, "help") == 0) {
    cmd_help();
  } else if (strcmp(line, "status") == 0) {
    cmd_status();
  } else if (strcmp(line, "led") == 0) {
    cmd_led(arg);
  } else {
    printf("unknown command: %s (try help)\r\n", line);
  }
}

void console_init(UART_HandleTypeDef *huart)
{
  s_uart = huart;
  s_head = 0;
  s_tail = 0;
  s_line_len = 0;
  HAL_UART_Receive_IT(s_uart, &s_rx_byte, 1);
}

void console_rx_complete(UART_HandleTypeDef *huart)
{
  if (huart != s_uart) {
    return;
  }
  uint32_t next = (s_head + 1u) % RX_QUEUE_SIZE;
  if (next != s_tail) { /* drop the byte when the queue is full */
    s_queue[s_head] = s_rx_byte;
    s_head = next;
  }
  HAL_UART_Receive_IT(s_uart, &s_rx_byte, 1);
}

void console_poll(void)
{
  while (s_tail != s_head) {
    uint8_t c = s_queue[s_tail];
    s_tail = (s_tail + 1u) % RX_QUEUE_SIZE;
    if (c == '\r' || c == '\n') {
      if (s_line_len > 0) {
        s_line[s_line_len] = '\0';
        dispatch(s_line);
        s_line_len = 0;
      }
    } else if (c == 0x08 || c == 0x7f) { /* backspace */
      if (s_line_len > 0) {
        s_line_len--;
      }
    } else if (s_line_len < LINE_MAX && c >= 0x20 && c < 0x7f) {
      s_line[s_line_len++] = (char)c;
    }
  }
}
