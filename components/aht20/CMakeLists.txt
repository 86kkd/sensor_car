idf_component_register(SRCS "aht20.c"
                    PRIV_REQUIRES driver i2c_facter
                    INCLUDE_DIRS "./include")
