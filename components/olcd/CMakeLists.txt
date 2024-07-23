idf_component_register(SRCS "olcd.c" "lvgl_demo_ui.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES driver i2c_facter
)
