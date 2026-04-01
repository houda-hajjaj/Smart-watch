SET(SOURCES components/me_vue.c
    components/screens.c
    screens/screen_splash.c
    screens/screen_home.c
    screens/screen_weather.c
    screens/screen_compass.c
    screens/screen_activity.c
    screens/screen_ble.c
    components/styles.c
    ui.c
    ui_helpers.c
    ui_events.c
    images/ui_img_gear_icon_png.c
    images/ui_img_weather_png.c)

add_library(ui ${SOURCES})

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/components
    ${CMAKE_CURRENT_SOURCE_DIR}/screens
    ${CMAKE_CURRENT_SOURCE_DIR}/images)
