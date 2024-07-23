#include "esp_err.h"
#include "esp_log.h"
#define BH1750_DEVICE_ADDR (0x23)
#define I2C_BH1750_TIME (30)
// #define BH1750_ADDR_LO 0x23 //!< I2C address when ADDR pin floating/low
// #define BH1750_ADDR_HI 0x5c //!< I2C address when ADDR pin high

/**
 * Measurement mode
 */
typedef enum {
  BH1750_MODE_ONE_TIME = 0, //!< One time measurement
  BH1750_MODE_CONTINUOUS    //!< Continuous measurement
} bh1750_mode_t;

/**
 * Measurement resolution
 */
typedef enum {
  BH1750_RES_LOW = 0, //!< 4 lx resolution, measurement time is usually 16 ms
  BH1750_RES_HIGH,    //!< 1 lx resolution, measurement time is usually 120 ms
  BH1750_RES_HIGH2    //!< 0.5 lx resolution, measurement time is usually 120 ms
} bh1750_resolution_t;

esp_err_t bh1750_power_down();
/*
**
* @brief Power on device
*
* @param dev Pointer to device descriptor
* @return `ESP_OK` on success
*/
esp_err_t bh1750_power_on();

/**
 * @brief Setup device parameters
 *
 * @param dev Pointer to device descriptor
 * @param mode Measurement mode
 * @param resolution Measurement resolution
 * @return `ESP_OK` on success
 */
esp_err_t setup_bh1750();

/**
 * @brief Set measurement time
 *
 * @param dev Pointer to device descriptor
 * @param time Measurement time (see datasheet)
 * @return `ESP_OK` on success
 */
esp_err_t bh1750_set_measurement_time(uint8_t time);

/**
 * @brief Read LUX value from the device.
 *
 * @param dev Pointer to device descriptor
 * @param[out] level read value in lux units
 * @return `ESP_OK` on success
 */
esp_err_t bh1750_read(uint8_t mode, uint8_t resolution, float *level);
