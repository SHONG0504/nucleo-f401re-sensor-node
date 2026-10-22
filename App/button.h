/* The user button B1 (PC13, active low, EXTI) toggles LD2 (PA5). The
 * interrupt debounces and records the press; the main loop acts on it. */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

void button_init(void);

/* Act on a recorded press. Call from the main loop. */
void button_poll(void);

void button_set_led(bool on);
bool button_led_is_on(void);

#endif
