
#include "esp_lvgl_port.h"
#include "lvgl.h"
typedef struct _olcd_data {
  lv_disp_t *disp;
  float *data;
} olcd_data;
lv_disp_t *setup_olcd(void);
void run_olcd(void *);
