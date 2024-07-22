/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_log.h"
#include "esp_log_level.h"

#include "lvgl.h"
#include "widgets/lv_label.h"
#include <stdint.h>
#include <stdio.h>

#define STRING ("Temp\t: %d°C\nRH\t: %d%%\nIllum\t: %d lx\nCO: %d ppm")

static const char *TAG = "LCD_UI";

void display_txt(lv_disp_t *disp, const char *text, int16_t x, int16_t y) {

  lv_obj_t *scr = lv_disp_get_scr_act(disp);
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, text);
  lv_label_set_long_mode(label,
                         LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
  // lv_obj_set_width(label, 150); /*Set smaller width to make the lines wrap*/
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  /* Size of the screen (if you use rotation 90 or 270, please set
   * disp->driver->ver_res) */
  lv_obj_set_width(label, disp->driver->hor_res);
  lv_obj_align(label, LV_ALIGN_CENTER, x, y);
}

void example_lvgl_demo_ui(lv_disp_t *disp) {

  // get pos and fromat string to print

  char *formatted_text = NULL;
  int len;
  len = lv_snprintf(NULL, 0, STRING, 36, 50, 10, 0);
  formatted_text = malloc(len + 10);
  if (formatted_text != NULL) {
    lv_snprintf(formatted_text, len + 10, STRING, 36, 50, 10, 0);
  }

  display_txt(disp, formatted_text, 0, 0);

  if (formatted_text != NULL) {
    free(formatted_text);
    formatted_text = NULL;
    ESP_LOGI(TAG, "free formatted_text");
  } else {
    ESP_LOGW(TAG, "there must be smomething wrong free null pointer");
  }
}
