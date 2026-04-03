#ifndef ZSWATCH_VIEW_SCREENS_H
#define ZSWATCH_VIEW_SCREENS_H

#include <lvgl.h>

#include "me_vue.h"

#ifdef __cplusplus
extern "C" {
#endif

int view_screens_init(const view_config_t *config);
void view_screens_deinit(void);
lv_obj_t *view_screens_get(view_screen_id_t id);
void view_screens_update(const view_model_data_t *model);

#ifdef __cplusplus
}
#endif

#endif
