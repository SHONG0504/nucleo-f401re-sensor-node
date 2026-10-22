/* The sensor input: ADC1 channel 0 (PA0, Arduino A0 on the Nucleo), read
 * once a second and reported on the console in millivolts. */
#ifndef SAMPLER_H
#define SAMPLER_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#define SAMPLER_PERIOD_MS 1000u
#define SAMPLER_VREF_MV   3300u
#define SAMPLER_FULL_SCALE 4095u /* 12-bit */

void sampler_init(ADC_HandleTypeDef *hadc);

/* Take a sample when the period is due. Call from the main loop. */
void sampler_poll(uint32_t now_ms);

uint16_t sampler_last_mv(void);
uint32_t sampler_count(void);

/* The last SAMPLER_HISTORY samples, as millivolts. */
#define SAMPLER_HISTORY 32u

typedef struct {
  uint16_t count; /* samples in the window, up to SAMPLER_HISTORY */
  uint16_t min_mv;
  uint16_t max_mv;
  uint16_t avg_mv;
} sampler_stats_t;

/* false while there is no sample yet. */
bool sampler_stats(sampler_stats_t *out);

#endif
