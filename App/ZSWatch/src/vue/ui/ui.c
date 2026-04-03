#include "ui.h"

#include <lvgl.h>

int ui_init_with_config(const view_config_t *config)
{
    return ME_VUE_init(config);
}

void ui_init(void)
{
    (void)ME_VUE_init((const view_config_t *)0);
}

void ui_destroy(void)
{
    ME_VUE_deinit();
}

void ui_process(void)
{
    (void)lv_timer_handler();
    ME_VUE_process();
}

void ui_update(const view_model_data_t *model)
{
    ME_VUE_update(model);
}
