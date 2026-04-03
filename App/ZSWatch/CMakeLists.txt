cmake_minimum_required(VERSION 3.20.0)
# set(SHIELD x_nucleo_iks01a3)
# set(SHIELD adafruit_2_8_tft_touch_shield_v2)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ZSWatch)

FILE(GLOB_RECURSE app_sources CONFIGURE_DEPENDS src/*.c)
target_sources(app PRIVATE ${app_sources})

target_include_directories(app PRIVATE
    src
    src/model
    src/model/sensors
    src/model/processing
    src/model/rtc
    src/controller
    src/controller/ble
    src/controller/input
    src/controller/thread
    src/vue
    src/vue/ui
    src/vue/ui/components
    src/vue/ui/screens
)
