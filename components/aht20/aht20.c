#include "aht20.h"
#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_facter.h"

#define AHT20_READ_ADDR ((AHT20_DEVICE_ADDR << 1) | 1)
#define AHT20_WRITE_ADDR ((AHT20_DEVICE_ADDR << 1) | 0)

#define GET_BIT(byte, index) ((byte & (1 << index)) >> index)

//////////////////////////////////////////////// GLOBAL_VARIABLE
///////////////////////////////////////////////////

static const char *TAG = "AHT20";

uint8_t data[7] = {0};
uint8_t busy;
uint8_t mode;
uint8_t cal;

extern i2c_master_bus_handle_t i2c_bus;
i2c_master_dev_handle_t i2c_dev_handle_aht20 = NULL;

//////////////////////////////////////////////// PRIVATE_DEFINITION
///////////////////////////////////////////////////

/// @brief send init command to device, in order to init the device
/// @return esp_err_t for result
static esp_err_t m_AHT20_command_init(void);

/// @brief send reset command to device, in order to restart the device
/// @return esp_err_t for result
static esp_err_t m_AHT20_command_reset(void);

/// @brief send measure command to device
/// @return esp_err_t for result
static esp_err_t m_AHT20_command_measure(void);

/// @brief get the status of device
/// @param pAHT20_data a pointer to the struct of data
/// @return esp_err_t for result
static esp_err_t m_AHT20_get_status(AHT20_data_t *pAHT20_data);

/// @brief send a read command and get result form device, the data will be
/// storaged in data[]
/// @param pAHT20_data a pointer to the struct of data
/// @return esp_err_t for result
static esp_err_t m_AHT20_get_result(AHT20_data_t *pAHT20_data);

//////////////////////////////////////////////// PRIVATE_DECLARATION
///////////////////////////////////////////////////

esp_err_t setup_AHT20() {
  esp_err_t ret;
  i2c_device_config_t dev_config1 = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = AHT20_DEVICE_ADDR, // 从设备1的7位I2C地址
      .scl_speed_hz = I2C_MASTER_FREQ_HZ};
  ret = i2c_master_bus_add_device(i2c_bus, &dev_config1, &i2c_dev_handle_aht20);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device 1: %s", esp_err_to_name(ret));
    i2c_del_master_bus(i2c_bus);
    return ret;
  } else {
    return ret;
  }
}

/// @brief send init command to device, in order to init the device
/// @return esp_err_t for result
static esp_err_t m_AHT20_command_init(void) {
  esp_err_t ret;
  uint8_t init_cmd[] = {0xBE, 0x08, 0x00};

  ret = i2c_master_transmit(i2c_dev_handle_aht20, init_cmd, sizeof(init_cmd),
                            100);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send init command: %s", esp_err_to_name(ret));
  }
  ESP_LOGD("m_AHT20_command_init", "already send INIT code");
  return ret;
}

/// @brief send reset command to device, in order to restart the device
/// @return esp_err_t for result
static esp_err_t m_AHT20_command_reset(void) {
  esp_err_t ret;
  uint8_t reset_cmd[] = {0xBA};

  ret = i2c_master_transmit(i2c_dev_handle_aht20, reset_cmd, sizeof(reset_cmd),
                            portMAX_DELAY);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send reset command: %s", esp_err_to_name(ret));
  }
  ESP_LOGD("m_AHT20_command_reset", "already send RESET code");
  return ret;
}

/// @brief send measure command to device
/// @return esp_err_t for result
static esp_err_t m_AHT20_command_measure(void) {
  esp_err_t ret;
  uint8_t measure_cmd[] = {0xAC, 0x33, 0x00};

  ret = i2c_master_transmit(i2c_dev_handle_aht20, measure_cmd,
                            sizeof(measure_cmd), portMAX_DELAY);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send measure command: %s", esp_err_to_name(ret));
  }
  ESP_LOGD("m_AHT20_command_measure", "already send MEASURES code");
  return ret;
}

/// @brief get the status of device
/// @param pAHT20_data a pointer to the struct of data
/// @return esp_err_t for result
static esp_err_t m_AHT20_get_status(AHT20_data_t *pAHT20_data) {
  esp_err_t ret = 1;

  // if bit[7] == 1, device is busy
  if (GET_BIT(data[0], 7) == 1)
    busy = 1;
  else
    busy = 0;

  // if bit[3] == 1, device has been calibrated
  if (GET_BIT(data[0], 3) == 1)
    cal = 1;
  else
    cal = 0;

  // if bit[6] == 1, device is under CMD mode
  if (GET_BIT(data[0], 6) == 1)
    mode = 2;
  // if bit[5] == 1 and bit[6] == 0, device is under CYC mode
  else if (GET_BIT(data[0], 6) == 0 || GET_BIT(data[0], 5) == 1)
    mode = 1;
  // if bit[5] == 0 and bit[6] == 0, device is under NOR mode
  else
    mode = 0;

  ESP_LOGD(
      "m_AHT20_get_status",
      "AHT20 STATUS: BUSY=%d; CAL=%d; MODE=%d; BIN:%d%d%d%d%d%d%d%d; HEX:%x",
      busy, cal, mode, GET_BIT(data[0], 0), GET_BIT(data[0], 1),
      GET_BIT(data[0], 2), GET_BIT(data[0], 3), GET_BIT(data[0], 4),
      GET_BIT(data[0], 5), GET_BIT(data[0], 6), GET_BIT(data[0], 7), data[0]);

  ret = 0;
  return ret;
}

/// @brief send a read command and get result form device, the data will be
/// storaged in data[]
/// @param pAHT20_data a pointer to the struct of data
/// @return esp_err_t for result
static esp_err_t m_AHT20_get_result(AHT20_data_t *pAHT20_data) {
  esp_err_t ret;

  ret = i2c_master_receive(i2c_dev_handle_aht20, data, sizeof(data),
                           portMAX_DELAY);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get result: %s", esp_err_to_name(ret));
  }
  ESP_LOGD("m_AHT20_get_result", "already get the result");

  return ret;
}

//////////////////////////////////////////////// PUBLIC_DECLARATION
///////////////////////////////////////////////////

/// @brief measure the temp and rh
/// @param pAHT20_data pointer of struct
/// @return esp_err_t for result
esp_err_t AHT20_measure(AHT20_data_t *pAHT20_data) {
  esp_err_t ret = 1;
  uint32_t rh, temp = 0;
  float rh_buf, temp_buf = 0;

  vTaskDelay(40 / portTICK_PERIOD_MS);
  m_AHT20_command_measure();
  vTaskDelay(75 / portTICK_PERIOD_MS);
  m_AHT20_get_result(pAHT20_data);
  m_AHT20_get_status(pAHT20_data);

  // continue if busy
  if (busy == 1) {
    ESP_LOGI("AHT20_task", "DEVICE IS BUSY");
    return ret;
  }

  // if cal == 0, reset the device
  if (cal == 0) {
    vTaskDelay(40 / portTICK_PERIOD_MS);
    m_AHT20_command_reset();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    m_AHT20_command_init();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    ESP_LOGI("AHT20_task", "NOT CAL OR DEVICE IS NOT CONNECTED");
    return ret;
  }

  rh = (((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (data[3])) >> 4;
  temp = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) |
         (uint32_t)data[5];

  rh_buf = (rh * (0.0000953674316F));
  temp_buf = (temp * (0.00019073F) - 50);

  // check the measurement results and reset the device if it is out of range
  if (pAHT20_data->RH == rh_buf || pAHT20_data->TEMP == temp_buf ||
      rh_buf > 85 || rh_buf < 15 || temp_buf > 95 || temp_buf < -18) {
    vTaskDelay(40 / portTICK_PERIOD_MS);
    m_AHT20_command_reset();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    m_AHT20_command_init();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    ESP_LOGI("AHT20_task", "OUT OF RANGE");
    return ret;
  }

  pAHT20_data->RH = rh_buf;
  pAHT20_data->TEMP = temp_buf;

  ESP_LOGI("AHT20_task", "TEMP: %f; RH: %f", pAHT20_data->TEMP,
           pAHT20_data->RH);
  ret = 0;
  return ret;
}
