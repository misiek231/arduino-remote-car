#include "camera_app.h"

extern constexpr ledc_mode_t FLASH_MODE = LEDC_LOW_SPEED_MODE;
extern constexpr ledc_timer_t FLASH_TIMER = LEDC_TIMER_1;
extern constexpr ledc_channel_t FLASH_CHANNEL = LEDC_CHANNEL_2;

bool setFlash(int percent) {
  if (!flashReady) return false;
  const uint32_t duty = (static_cast<uint32_t>(percent) * 255 + 50) / 100;
  if (ledc_set_duty(FLASH_MODE, FLASH_CHANNEL, duty) != ESP_OK ||
      ledc_update_duty(FLASH_MODE, FLASH_CHANNEL) != ESP_OK) return false;
  flashPercent = percent;
  return true;
}

bool initFlash() {
  ledc_timer_config_t timer = {};
  timer.speed_mode = FLASH_MODE;
  timer.duty_resolution = LEDC_TIMER_8_BIT;
  timer.timer_num = FLASH_TIMER; // Kamera ma osobny timer 0.
  timer.freq_hz = 5000;
  timer.clk_cfg = LEDC_AUTO_CLK;
  if (ledc_timer_config(&timer) != ESP_OK) return false;
  ledc_channel_config_t channel = {};
  channel.gpio_num = FLASH_PIN;
  channel.speed_mode = FLASH_MODE;
  channel.channel = FLASH_CHANNEL; // Kamera ma kanal 0.
  channel.timer_sel = FLASH_TIMER;
  channel.duty = 0;
  channel.hpoint = 0;
  return ledc_channel_config(&channel) == ESP_OK;
}

