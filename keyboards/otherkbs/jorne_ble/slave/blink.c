/*
Copyright 2018 Sekigon

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "matrix.h"

#include "wait.h"
#include "app_ble_func.h"
#include "bootloader.h"

#ifdef SSD1306OLED
  #include "ssd1306.h"
#endif

#include "nrf_power.h"
#include "nrf.h"

#include "nrf_gpio.h"
#include "nrf_delay.h"

#ifdef RGBLIGHT_ENABLE
#include "rgblight.h"
extern rgblight_config_t rgblight_config;
#endif


void unselect_rows(void);
void select_row(uint8_t row);
matrix_row_t read_cols(void);
static bool bootloader_flag = false;

int pins[]={PIN1, PIN2, PIN5, PIN6, PIN7, PIN8, PIN9, PIN10, PIN11, PIN12, PIN13, PIN14, PIN15, PIN16, PIN17, PIN18, PIN19, PIN20};

void set_pin(int pin, int set) {
    if (set) nrf_gpio_pin_set(pin);
    else nrf_gpio_pin_clear(pin);
}

void matrix_init_user() {

  // blink on power on
  nrf_gpio_cfg_output(LED_PIN);
  nrf_gpio_cfg_input(SWITCH_PIN, NRF_GPIO_PIN_PULLDOWN);

    int pin_count = sizeof(pins)/sizeof(pins[0]);

    for (int j=0; j<pin_count; j++) nrf_gpio_cfg_output(pins[j]);

  for (int i = 0; i < 1000; i++) {
    nrf_gpio_pin_set(LED_PIN);
    
    for (int j=0; j<pin_count; j++) set_pin(pins[j], 1);

    nrf_delay_ms(100);

    nrf_gpio_pin_clear(LED_PIN);

    for (int j=0; j<pin_count; j++) set_pin(pins[j], 0);

    nrf_delay_ms(100);
  }

  nrf_gpio_pin_set(LED_PIN);

#ifdef RGBLIGHT_ENABLE
  // turn on RGB leds by default, debug option *remove me*
  // mode change doesnt work until you press bl reset (adjust+lrst)
  eeconfig_update_rgblight_default();
  rgblight_enable();
#endif

  //SSD1306 OLED init, make sure to add #define SSD1306OLED in config.h
  #ifdef SSD1306OLED
      iota_gfx_init(!IS_LEFT_HAND);   // turns on the display
  #endif

  select_row(3);
  wait_us(50);
  matrix_row_t row = read_cols();
  unselect_rows();
  if (row == 0b111000) {
    delete_bonds();
  } else if (row == 0b10) {
    bootloader_flag = true;
  }
}

void matrix_scan_user() {
 static int cnt;
 if (bootloader_flag && cnt++==500) {
   bootloader_jump();
 }
#ifdef SSD1306OLED
  iota_gfx_task();  // this is what updates the display continuously
#endif
}

