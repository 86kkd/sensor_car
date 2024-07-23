#include "esp_err.h"
#include "esp_log.h"
#include "olcd.h"
#include <aht20.h>
#include <i2c_facter.h>
#include <stdio.h>

static const char *TAG = "MAIN";

void app_main(void) {

  // vector to store sensor data
  int data[5];

  // setup i2c bus
  setup_device_i2c();

  // setup aht20
  setup_AHT20();

  // setup olcd
  lv_disp_t *disp = NULL;
  disp = setup_olcd();

  data[0] = 5;
  data[1] = 37;
  data[2] = 50;
  data[3] = 10;
  data[4] = 0;

  AHT20_data_t AHT20_data = {.RH = 0, .TEMP = 0};

  olcd_data lcd_data = {disp, data};
  xTaskCreatePinnedToCore(run_olcd, "LVGL_UI", 4096, &lcd_data, 1, NULL, 0);
  // run_olcd(&lcd_data);
  while (1) {
    ESP_LOGI(TAG, "run main while");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_err_t ret = AHT20_measure(&AHT20_data);
    data[1] = (int)AHT20_data.TEMP;
    data[2] = (int)AHT20_data.RH;
    if (ret == 0)
      printf("RH: %.2f; TEMP: %.2f\n", AHT20_data.RH, AHT20_data.TEMP);

    // data[1]++;
    if (data[1] == 100)
      data[1] = 0;
  }
}
