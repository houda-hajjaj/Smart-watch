# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10.0)

set(SHIELD adafruit_2_8_tft_touch_v2)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_input_tsc2007)

target_sources(app PRIVATE src/main.c)
