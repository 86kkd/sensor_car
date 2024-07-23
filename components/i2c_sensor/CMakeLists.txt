idf_component_register(SRCS "i2c_sensor.c" 
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_i2c driver
)
