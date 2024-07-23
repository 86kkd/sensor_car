idf_component_register(SRCS "bh1750.c"
                    PRIV_REQUIRES driver i2c_facter
                    INCLUDE_DIRS "./include")
