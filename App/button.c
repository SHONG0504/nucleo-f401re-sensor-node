#include "button.h"

#include <stdio.h>

#include "main.h"

/* How long the contacts bounce after a press, from the bench: well under 10 ms
 * on this board, 50 ms leaves margin for older switches. */
#define BUTTON_DEBOUNCE_MS 50u

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

/* HAL calls this from EXTI15_10_IRQHandler for every EXTI line that fired.
 * The button bounces for a few milliseconds, so wait out the bounce and only
 * count the press if the pin is still low afterwards. */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin != B1_Pin) {
    return;
  }
  HAL_Delay(BUTTON_DEBOUNCE_MS);
  if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
    s_pressed = true;
  }
}
