#include "esp_err.h"
#include "esp_log.h"
#include "olcd.h"
#include <aht20.h>
#include <stdio.h>

#define I2C_MASTER_SDA_IO 1
#define I2C_MASTER_SCL_IO 2
#define I2C_MASTER_FREQ_HZ 100 * 1000

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
  // I2C设备配置，这里以AHT20为例，实际设备地址需要根据数据手册确定
  i2c_device_config_t i2c_dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7, // 7位设备地址
      .device_address = 0x38,                // AHT20的7位I2C地址
      .scl_speed_hz = I2C_MASTER_FREQ_HZ     // I2C时钟频率
  };
  AHT20_data_t AHT20_data = {.RH = 0, .TEMP = 0};

  // i2c_param_config (I2C_MASTER_PORT, &conf);
  // i2c_driver_install(I2C_MASTER_PORT, I2C_MODE_MASTER, 0, 0, 0);

  olcd_data lcd_data = {disp, data};
  xTaskCreatePinnedToCore(run_olcd, "LVGL_UI", 4096, &lcd_data, 1, NULL, 0);
  // run_olcd(&lcd_data);
  while (1) {
    ESP_LOGI(TAG, "run main while");
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    data[1] = (int)AHT20_data.TEMP;
    data[2] = (int)AHT20_data.RH;
    esp_err_t ret = AHT20_measure(&AHT20_data);
    if (ret == 0)
      printf("RH: %.2f; TEMP: %.2f\n", AHT20_data.RH, AHT20_data.TEMP);

    // data[1]++;
    if (data[1] == 100)
      data[1] = 0;
  }
}
