/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "core/lv_obj.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include <esp_lvgl_port.h>

#include "esp_task.h"
#include "hal/lv_hal_disp.h"
#include "lvgl.h"
#include "widgets/lv_label.h"
#include <stdint.h>
#include <stdio.h>

#define STRING ("Temp\t: %.2f°C\nRH\t: %.2f%%\nIllum\t: %.1f lx\nCO: %.2f ppm")

static const char *TAG = "LCD_UI";

typedef struct _label_data {
  lv_obj_t *label;
  float *data;
} label_data;

lv_obj_t *display_txt(lv_disp_t *disp, float *data, int16_t x, int16_t y) {

  lv_obj_t *scr = lv_disp_get_scr_act(disp);
  lv_obj_t *label = lv_label_create(scr);
  // lv_label_set_text_fmt(label, STRING, data[1], data[2], data[3], data[4]);
  char buf[128];
  snprintf(buf, sizeof(buf), STRING, data[1], data[2], data[3], data[4]);
  lv_label_set_text(label, buf);
  lv_label_set_long_mode(label,
                         LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
  // lv_obj_set_width(label, 150); /*Set smaller width to make the lines wrap*/
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  /* Size of the screen (if you use rotation 90 or 270, please set
   * disp->driver->ver_res) */
  lv_obj_set_width(label, disp->driver->hor_res);
  lv_obj_align(label, LV_ALIGN_CENTER, x, y);
  return label;
}

static void update_data_cb(lv_timer_t *timer) {
  ESP_LOGI(TAG, "lcad thread is running");
  label_data *l_data = (label_data *)timer->user_data; // 更新标签的文本
  lv_obj_t *label = l_data->label;
  float *data = l_data->data;
  // lv_label_set_text_fmt(label, STRING, data[1], data[2], data[3], data[4]);
  // set_text_fmt can't print float so kicked
  char buf[128];
  snprintf(buf, sizeof(buf), STRING, data[1], data[2], data[3], data[4]);
  lv_label_set_text(label, buf);
}

void lvgl_ui(lv_disp_t *disp, float *data) {

  // get pos and fromat string to print
  ESP_LOGI(TAG, "start display");
  lv_obj_t *label;
  label = display_txt(disp, data, 0, 0);
  label_data l_data = {label, data};
  lv_timer_t *timer = lv_timer_create(update_data_cb, 500, &l_data);
  lv_timer_set_repeat_count(timer, -1);
  while (1) {
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(0)) {
      lv_task_handler();
      // Release the mutex
      lvgl_port_unlock();
    }
  }
}
