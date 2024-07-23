set(component_srcs "aht20.c")
idf_component_register(SRCS "${component_srcs}"
                    PRIV_REQUIRES driver i2c_facter
                    INCLUDE_DIRS "./include")
