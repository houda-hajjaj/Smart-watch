#include <zephyr/kernel.h>

#include "input/buttons.h"
#include "thread/data_init_thread.h"
#include "thread/sampling_thread.h"
#include "thread/ble_thread.h"
#include "thread/rtc_thread.h"
#include "thread/ui_thread.h"
#include "thread/power_thread.h"

int main(void)
{
	(void)buttons_init();
	data_init_thread_start();
	ui_thread_start();
	k_sleep(K_MSEC(100));
	sampling_thread_start();
	ble_thread_start();
	rtc_thread_start();
	power_thread_start();

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}
