#include "esp_err.h"
#include "esp_log.h"
#include "olcd.h"
#include <stdio.h>

static const char *TAG = "MAIN";

void app_main(void) {
  lv_disp_t *disp = NULL;
  disp = setup_olcd();

  int data[5];
  data[0] = 5;
  data[1] = 37;
  data[2] = 50;
  data[3] = 10;
  data[4] = 0;
  olcd_data lcd_data = {disp, data};
  xTaskCreatePinnedToCore(run_olcd, "LVGL_UI", 4096, &lcd_data, 1, NULL, 0);
  // run_olcd(&lcd_data);
  while (1) {
    ESP_LOGI(TAG, "run main while");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    data[1]++;
    if (data[1] == 100)
      data[1] = 0;
  }
}
