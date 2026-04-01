cmake_minimum_required(VERSION 3.20.0)
# set(SHIELD x_nucleo_iks01a3)
# set(SHIELD adafruit_2_8_tft_touch_shield_v2)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ZSWatch)

FILE(GLOB_RECURSE app_sources CONFIGURE_DEPENDS src/*.c)
target_sources(app PRIVATE ${app_sources})

target_include_directories(app PRIVATE
    src
    src/sensors
    src/processing
    src/ble
    src/thread
    src/ui
    src/ui/components
    src/ui/screens
    src/ui/images
)
