#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_facter.h"
#include <bh1750.h>
#include <stdint.h>
#include <unistd.h>

#define OPCODE_HIGH 0x0
#define OPCODE_HIGH2 0x1
#define OPCODE_LOW 0x3

#define OPCODE_CONT 0x10
#define OPCODE_OT 0x20

#define OPCODE_POWER_DOWN 0x00
#define OPCODE_POWER_ON 0x01
#define OPCODE_MT_HI 0x40
#define OPCODE_MT_LO 0x60

static const char *TAG = "bh1750";

extern i2c_master_bus_handle_t i2c_bus;
i2c_master_dev_handle_t i2c_dev_handle_bh1750 = NULL;

esp_err_t bh1750_power_down() {

  uint8_t cmd = OPCODE_POWER_DOWN;

  return i2c_master_transmit(i2c_dev_handle_bh1750, &cmd, sizeof(cmd),
                             portMAX_DELAY);
}

esp_err_t bh1750_power_on() {

  uint8_t cmd = OPCODE_POWER_ON;

  return i2c_master_transmit(i2c_dev_handle_bh1750, &cmd, sizeof(cmd),
                             portMAX_DELAY);
}

esp_err_t setup_bh1750() {

  esp_err_t ret;
  i2c_device_config_t dev_config1 = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = BH1750_DEVICE_ADDR, // 从设备1的7位I2C地址
      .scl_speed_hz = I2C_MASTER_FREQ_HZ};
  ret =
      i2c_master_bus_add_device(i2c_bus, &dev_config1, &i2c_dev_handle_bh1750);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device 1: %s", esp_err_to_name(ret));
    i2c_del_master_bus(i2c_bus);
    return ret;
  } else {
    ESP_LOGI(TAG, "Success add I2C device : %s", esp_err_to_name(ret));
    return ret;
  }

  return ESP_OK;
}

esp_err_t bh1750_set_measurement_time(uint8_t time) {

  uint8_t cmd[] = {OPCODE_MT_HI | (time >> 5), OPCODE_MT_LO | (time & 0x1f)};

  esp_err_t ret = i2c_master_transmit(i2c_dev_handle_bh1750, cmd, sizeof(cmd),
                                      portMAX_DELAY);
  if (ret != ESP_OK) {
    // 处理错误
    ESP_LOGE(TAG, "i2c_master_transmit failed: %s", esp_err_to_name(ret));
    return ret;
  }
  return ret;
}

esp_err_t bh1750_read(uint8_t mode, uint8_t resolution, float *level) {
  esp_err_t ret;
  uint8_t opcode = mode == BH1750_MODE_CONTINUOUS ? OPCODE_CONT : OPCODE_OT;
  switch (resolution) {
  case BH1750_RES_LOW:
    opcode |= OPCODE_LOW;
    break;
  case BH1750_RES_HIGH:
    opcode |= OPCODE_HIGH;
    break;
  default:
    opcode |= OPCODE_HIGH2;
    break;
  }
  ret = i2c_master_transmit(i2c_dev_handle_bh1750, &opcode, sizeof(opcode),
                            portMAX_DELAY);
  if (ret != ESP_OK) {
    // 处理错误
    ESP_LOGE(TAG, "i2c_master_transmit failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "bh1750_setup(PORT = %d, ADDR = 0x%02x, VAL = 0x%02x)",
           I2C_MASTER_PORT, BH1750_DEVICE_ADDR, opcode);

  vTaskDelay(I2C_BH1750_TIME / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "finish measure");

  uint8_t buf[2];

  ret = i2c_master_receive(i2c_dev_handle_bh1750, buf, sizeof(buf),
                           portMAX_DELAY);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get result: %s", esp_err_to_name(ret));
  }
  ESP_LOGD("m_AHT20_get_result", "already get the result");

  *level = (float)(buf[0] << 8 | buf[1]);
  *level = (float)((*level * 10) / 12); // convert to LUX

  return ESP_OK;
}
