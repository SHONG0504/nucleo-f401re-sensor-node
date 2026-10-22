#include "sampler.h"

#include <stdio.h>

static ADC_HandleTypeDef *s_adc;
static uint32_t s_next_ms;
static uint16_t s_last_mv;
static uint32_t s_count;

/* Ring of the most recent readings, for stats. */
static uint16_t s_history[SAMPLER_HISTORY];
static uint16_t s_history_len;
static uint16_t s_history_next;

static void history_push(uint16_t mv)
{
  s_history[s_history_next] = mv;
  s_history_next = (uint16_t)((s_history_next + 1u) % SAMPLER_HISTORY);
  if (s_history_len < SAMPLER_HISTORY) {
    s_history_len++;
  }
}

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
  s_history_len = 0;
  s_history_next = 0;
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
  history_push(s_last_mv);
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

bool sampler_stats(sampler_stats_t *out)
{
  if (s_history_len == 0) {
    return false;
  }
  uint16_t min = 0xFFFFu;
  uint16_t max = 0;
  uint16_t sum = 0;
  for (uint16_t i = 0; i < s_history_len; i++) {
    uint16_t mv = s_history[i];
    if (mv < min) {
      min = mv;
    }
    if (mv > max) {
      max = mv;
    }
    sum += mv;
  }
  out->count = s_history_len;
  out->min_mv = min;
  out->max_mv = max;
  out->avg_mv = (uint16_t)(sum / s_history_len);
  return true;
}
