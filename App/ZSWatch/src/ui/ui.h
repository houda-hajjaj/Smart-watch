#ifndef ZSWATCH_PORTABLE_UI_H
#define ZSWATCH_PORTABLE_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "components/me_vue.h"
#include "ui_events.h"
#include "ui_helpers.h"

int ui_init_with_config(const view_config_t *config);
void ui_init(void);
void ui_destroy(void);
void ui_process(void);
void ui_update(const view_model_data_t *model);

#ifdef __cplusplus
}
#endif

#endif /* ZSWATCH_PORTABLE_UI_H */
