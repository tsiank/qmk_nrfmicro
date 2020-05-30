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

#include "ble_master.h"
#include "ble_central.h"
#include "app_ble_func.h"
#include "usbd.h"

/* TMK includes */
#include "report.h"
#include "host.h"
#include "host_driver.h"
#include "keyboard.h"
#include "action.h"
#include "action_util.h"
#include "mousekey.h"
#include "led.h"
#include "sendchar.h"
#include "debug.h"
#include "printf.h"
#ifdef SLEEP_LED_ENABLE
#include "sleep_led.h"
#endif
#ifdef SERIAL_LINK_ENABLE
#include "serial_link/system/serial_link.h"
#endif
#ifdef VISUALIZER_ENABLE
#include "visualizer/visualizer.h"
#endif
#ifdef MIDI_ENABLE
#include "qmk_midi.h"
#endif
#ifdef RGB_MATRIX_ENABLE
#include "rgb_matrix.h"
#endif
#ifdef RGBLIGHT_ENABLE
#include "rgblight.h"
#endif
#include "suspend.h"
#include "wait.h"

const uint8_t MAINTASK_INTERVAL=17;

#ifndef KEYBOARD_SCAN_INTERVAL
#    define KEYBOARD_SCAN_INTERVAL 4
#endif
/* -------------------------
 *   TMK host driver defs
 * -------------------------
 */

/* declarations */
uint8_t keyboard_leds(void);
void send_keyboard(report_keyboard_t *report);
void send_mouse(report_mouse_t *report);
void send_system(uint16_t data);
void send_consumer(uint16_t data);
void sendchar_pf(void *p, char c);

/* host struct */
host_driver_t nrf_ble_driver = {
  keyboard_leds,
  send_keyboard,
  send_mouse,
  send_system,
  send_consumer
};

#ifdef VIRTSER_ENABLE
void virtser_task(void);
#endif

#ifdef RAW_HID_ENABLE
void raw_hid_task(void);
#endif

#ifdef CONSOLE_ENABLE
void console_task(void);
#endif

#ifdef RGBLIGHT_ENABLE
#include "rgblight.h"
#ifdef RGBLIGHT_SPLIT
rgblight_syncinfo_t rgblight_sync;
#endif
#endif

uint32_t ble_nus_send_bytes_to_slave(uint8_t* buf, uint16_t len);

void eeprom_update(void);
void timer_tick(uint8_t interval);
void main_tasks(void* p_context) {
  UNUSED_PARAMETER(p_context);
  /* Main loop */
  timer_tick(MAINTASK_INTERVAL);

  keyboard_task();
#ifdef CONSOLE_ENABLE
  console_task();
#endif
#ifdef VIRTSER_ENABLE
  virtser_task();
#endif
#ifdef RAW_HID_ENABLE
  raw_hid_task();
#endif

#ifdef RGB_MATRIX_ENABLE
rgb_matrix_task();
#endif

#if defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_ANIMATIONS)
  rgblight_task();

#ifdef RGBLIGHT_SPLIT
  if (rgblight_get_change_flags()) {
    rgblight_get_syncinfo(&rgblight_sync);
    ble_nus_send_bytes_to_slave((uint8_t*)&rgblight_sync, sizeof(rgblight_sync));
    rgblight_clear_change_flags();
  }
#endif

#endif

  eeprom_update();
}

void send_keyboard(report_keyboard_t *report) {
  if (get_ble_enabled()) {
    ble_send_keyboard(report);
  }
  if (get_usb_enabled()) {
    usbd_send_keyboard(report);
  }
}

void send_mouse(report_mouse_t *report) {
  if (get_ble_enabled()) {
    ble_send_mouse(report);
  }
  if (get_usb_enabled()) {
    usbd_send_mouse(report);
  }
}

void send_system(uint16_t data) {
  if (get_ble_enabled()) {
    ble_send_system(data);
  }
  if (get_usb_enabled()) {
    usbd_send_system(data);
  }
}

void send_consumer(uint16_t data) {
  if (get_ble_enabled()) {
    ble_send_consumer(data);
  }
  if (get_usb_enabled()) {
    usbd_send_consumer(data);
  }
}

void send_abs_mouse(int8_t x, int8_t y) {
  if (get_ble_enabled()) {
    ble_send_abs_mouse(x, y);
  }
  if (get_usb_enabled()) {
    usbd_send_abs_mouse(x, y);
  }
}

/**@brief Function for application main entry.
 */
int main(void) {
//  // Initialize.
  logger_init();
  timers_init(main_tasks);

  usbd_init();

//app_usbd_enable();
//NRF_LOG_INFO("USB enable");
  adc_init();

#ifdef NRF_SEPARATE_KEYBOARD_MASTER
#endif
  ble_stack_init();
  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
  peer_manager_init();
  scheduler_init();
  gap_params_init();
  advertising_init();
  services_init();
  conn_params_init();

#ifdef NRF_SEPARATE_KEYBOARD_MASTER
  db_discovery_init();
  nus_c_init();
#endif
  // Start execution.
#if defined(ENABLE_STARTUP_ADV_LIST)
  advertising_start();
#elif defined(ENABLE_STARTUP_ADV_NOLIST)
  restart_advertising_wo_whitelist();
#endif
#ifdef NRF_SEPARATE_KEYBOARD_MASTER
  scan_start();
#endif
  timers_start();

  /* init printf */
  init_printf(NULL, sendchar_pf);

#ifdef MIDI_ENABLE
  setup_midi();
#endif

#ifdef SERIAL_LINK_ENABLE
  init_serial_link();
#endif

#ifdef VISUALIZER_ENABLE
  visualizer_init();
#endif


  host_driver_t* driver = NULL;
  driver = &nrf_ble_driver;

  /* Do need to wait here!
   * Otherwise the next print might start a transfer on console EP
   * before the USB is completely ready, which sometimes causes
   * HardFaults.
   */
  wait_ms(50);

  /* init TMK modules */
  keyboard_init();
  host_set_driver(driver);
  NRF_LOG_INFO("HID Keyboard Start!\r\n");

#ifdef SLEEP_LED_ENABLE
  sleep_led_init();
#endif

  main_task_start(MAINTASK_INTERVAL);
  usbd_enable();
  // Enter main loop.
  for (;;) {
    usbd_process();
    main_loop();
//    app_sched_execute();
//    if (NRF_LOG_PROCESS() == false) {
//      power_manage();
//    }
  }
}

#include "app_error.h"

#if NRF_SDK_MAJOR_VER==15
#include "nrf_strerror.h"
#define ERR_TO_STRING(err_code) nrf_strerror_get(err_code)
#else
#define ERR_TO_STRING(err_code) (uint32_t)ERR_TO_STR(err_code)
#endif

__WEAK void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
  error_info_t *err = (error_info_t*)info;
    NRF_LOG_ERROR("Fatal %d:%s %s:%d\r\n", err->err_code, ERR_TO_STRING(err->err_code), (uint32_t)err->p_file_name, err->line_num);
    NRF_LOG_FINAL_FLUSH();
    // On assert, the system can only recover with a reset.
#ifndef DEBUG
    NVIC_SystemReset();
#else
    while(1) { continue; }
    app_error_save_and_stop(id, pc, info);
#endif // DEBUG
}


/**
 * @}
 */
