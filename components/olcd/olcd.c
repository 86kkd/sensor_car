/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "olcd.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "i2c_facter.h"
#include "lvgl.h"
#include <stdio.h>

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#include "esp_lcd_sh1107.h"
#else
#include "esp_lcd_panel_vendor.h"
#endif

static const char *TAG = "LCD";

#define I2C_BUS_PORT 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your
/// LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_PIN_NUM_RST -1
#define EXAMPLE_I2C_HW_ADDR 0x3C

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
#define EXAMPLE_LCD_H_RES 128
#define EXAMPLE_LCD_V_RES CONFIG_EXAMPLE_SSD1306_HEIGHT
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#define EXAMPLE_LCD_H_RES 64
#define EXAMPLE_LCD_V_RES 128
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS 8
#define EXAMPLE_LCD_PARAM_BITS 8

extern void lvgl_ui(lv_disp_t *disp, int *data);
extern i2c_master_bus_handle_t i2c_bus;

lv_disp_t *setup_olcd(void) {
  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i2c_config_t io_config = {
      .dev_addr = EXAMPLE_I2C_HW_ADDR,
      .scl_speed_hz = I2C_MASTER_FREQ_HZ,
      .control_phase_bytes = 1,               // According to SSD1306 datasheet
      .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
      .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
      .dc_bit_offset = 6, // According to SSD1306 datasheet
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
      .dc_bit_offset = 0, // According to SH1107 datasheet
      .flags =
          {
              .disable_control_phase = 1,
          }
#endif
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

  ESP_LOGI(TAG, "Install SSD1306 panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;
  esp_lcd_panel_dev_config_t panel_config = {
      .bits_per_pixel = 1,
      .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
  };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
  esp_lcd_panel_ssd1306_config_t ssd1306_config = {
      .height = EXAMPLE_LCD_V_RES,
  };
  panel_config.vendor_config = &ssd1306_config;
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle));
#endif

  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif

  ESP_LOGI(TAG, "Initialize LVGL");
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  lvgl_port_init(&lvgl_cfg);

  const lvgl_port_display_cfg_t disp_cfg = {.io_handle = io_handle,
                                            .panel_handle = panel_handle,
                                            .buffer_size = EXAMPLE_LCD_H_RES *
                                                           EXAMPLE_LCD_V_RES,
                                            .double_buffer = true,
                                            .hres = EXAMPLE_LCD_H_RES,
                                            .vres = EXAMPLE_LCD_V_RES,
                                            .monochrome = true,
                                            .rotation = {
                                                .swap_xy = false,
                                                .mirror_x = false,
                                                .mirror_y = false,
                                            }};
  lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

  return disp;
}

void run_olcd(void *pvParameters) {
  /* Rotation of the screen */
  olcd_data *lcd_data = (olcd_data *)pvParameters;
  lv_disp_t *disp = lcd_data->disp;
  int *data = lcd_data->data;
  lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);
  ESP_LOGI(TAG, "Display LVGL Scroll Text");
  lvgl_ui(disp, data);
}
