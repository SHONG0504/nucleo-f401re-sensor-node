#include "sampler.h"

#include <stdio.h>

static ADC_HandleTypeDef *s_adc;
static uint32_t s_next_ms;
static uint16_t s_last_mv;
static uint32_t s_count;

/* One blocking conversion; the sequencer has only channel 0. */
static bool read_raw(uint16_t *raw)
{
  if (HAL_ADC_Start(s_adc) != HAL_OK) {
    return false;
  }
  if (HAL_ADC_PollForConversion(s_adc, 10) != HAL_OK) {
    HAL_ADC_Stop(s_adc);
    return false;
  }
  *raw = (uint16_t)HAL_ADC_GetValue(s_adc);
  HAL_ADC_Stop(s_adc);
  return true;
}

void sampler_init(ADC_HandleTypeDef *hadc)
{
  s_adc = hadc;
  s_next_ms = HAL_GetTick() + SAMPLER_PERIOD_MS;
  s_last_mv = 0;
  s_count = 0;
}

void sampler_poll(uint32_t now_ms)
{
  if ((int32_t)(now_ms - s_next_ms) < 0) {
    return;
  }
  s_next_ms += SAMPLER_PERIOD_MS;

  uint16_t raw;
  if (!read_raw(&raw)) {
    printf("sample: ADC error\r\n");
    return;
  }
  s_last_mv = (uint16_t)(((uint32_t)raw * SAMPLER_VREF_MV) / SAMPLER_FULL_SCALE);
  s_count++;
  printf("[%lu s] sample: %u mV\r\n", (unsigned long)(now_ms / 1000u), s_last_mv);
}

uint16_t sampler_last_mv(void)
{
  return s_last_mv;
}

uint32_t sampler_count(void)
{
  return s_count;
}
