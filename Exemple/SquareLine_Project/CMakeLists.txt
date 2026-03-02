# Copyright (c)  2024 LVGL <felipe@lvgl.io>
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(SHIELD adafruit_2_8_tft_touch_v2)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(SquareLine_Project)

set(LVGL_DIR ${ZEPHYR_LVGL_MODULE_DIR})

FILE(GLOB app_sources src/sensor/*.c src/*.c src/ui/*.c src/ui/images/*.c src/ui/fonts/*.c src/ui/screens/*.c src/ui/components/*.c)
target_sources(app PRIVATE ${app_sources})


if(CONFIG_DT_HAS_TI_TSC2007_ENABLED)
    target_sources(app PRIVATE ${ZEPHYR_BASE}/drivers/input/input_tsc2007.c)
endif()

