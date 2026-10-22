#include "button.h"

#include <stdio.h>

#include "main.h"

static volatile bool s_pressed;
static uint32_t s_presses;

void button_init(void)
{
  s_pressed = false;
  s_presses = 0;
  button_set_led(false);
}

void button_set_led(bool on)
{
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool button_led_is_on(void)
{
  return HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET;
}

void button_poll(void)
{
  if (!s_pressed) {
    return;
  }
  s_pressed = false;
  s_presses++;
  button_set_led(!button_led_is_on());
  printf("button: press %lu, LED %s\r\n", (unsigned long)s_presses, button_led_is_on() ? "on" : "off");
}

/* HAL calls this from EXTI15_10_IRQHandler for every EXTI line that fired. */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == B1_Pin) {
    s_pressed = true;
  }
}
