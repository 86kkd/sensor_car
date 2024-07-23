#include "i2c_facter.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "I2C_MASTER";
i2c_master_bus_handle_t i2c_bus = NULL;

void setup_device_i2c() {
  ESP_LOGI(TAG, "Initialize I2C bus");
  i2c_master_bus_config_t bus_config = {
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .i2c_port = I2C_MASTER_PORT,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .flags.enable_internal_pullup = true,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));
}
