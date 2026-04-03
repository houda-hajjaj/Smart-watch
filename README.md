# ZSWatch BLE Sensor Project


Notes: There are 2 `branches` in our project: 
- `main`branch: contains the main application code for the smartwatch prototype, including sensor integration, BLE services, and UI. All the code here is broken down into smaller parts for easier management, clarity, and maintainability.

- `Timer_X` branch: This code snippet is my attempt to optimize for the Model View Controller (MVC) model. The main idea is to separate the concerns of data acquisition (Model), user interface (View), and application logic (Controller) more clearly. In this branch, we have refactored the code to better align with MVC principles, which should improve modularity and maintainability.




#

This project implements a Bluetooth Low Energy (BLE) smartwatch prototype using an **nRF5340 DK** and an **ST IKS01A3 shield**. It runs on **Zephyr RTOS** (nRF Connect SDK v3.1.0) and demonstrates periodic sensor data transmission over BLE using standard GATT services.

## Features
- **Hardware**: nRF5340 DK (application core) + X-NUCLEO-IKS01A3 shield (LSM6DSO, LIS2MDL, HTS221, LPS22HH).
- **Sensor drivers**: Periodic reading of temperature, humidity, pressure, acceleration, and magnetic field.
- **BLE implementation**:
  - Environmental Sensing Service (ESS, UUID 0x181A) with standard characteristics:
    - Temperature (0x2A6E)
    - Humidity (0x2A6F)
    - Pressure (0x2A6D)
    - 3-axis accelerometer (0x2AA1)
    - 3-axis magnetometer (0x2AA0)
  - Notifications enabled for all characteristics.
  - Device name: "ZSWatch".
- **Robust communication**: Increased BLE buffers, proper CCC handling, and dual-core setup (network core runs `hci_ipc`).

```text
CMake arguments:
-DSHIELD=x_nucleo_iks01a3, adafruit_2_8_tft
```

## Status
- Sensors initialise correctly (I2C).
- BLE advertising and connection functional.
- Notifications sent every 2 seconds with scaled sensor values.
- Tested with nRF Connect for Mobile - data appears in real-time after subscribing.

# ZSWatch Project Log

This is a short log of the first development weeks.

## Week 1-2 - Initialize project and configuration

- Created the project structure and initialized the repository.
- Set up the Zephyr / nRF Connect SDK environment; added `CMakeLists.txt` and `prj.conf` for the `nrf5340dk` target.
- Performed an initial build with `west build` to verify the toolchain and resolve dependencies.
- Outcome: initial build succeeded and base configuration validated.

## Week 3-4 - Finish sensors and USART

- Integrated sensor drivers (altimeter, compass, `mag_sensor`, `env_sensor`, `motion_sensor`, `step_counter`) and configured I2C/SPI.
- Implemented periodic read routines and unit conversions for temperature, pressure, humidity, acceleration, and magnetism.
- Added a USART debug channel to log readings and simplify hardware debugging.
- Hardware tests confirmed stable readings and initial calibrations.

## Week 5-6 - BLE

- Implemented the main BLE services (Environmental Sensing Service and related characteristics).
- Enabled notifications to periodically send measurements to a connected client.
- Tuned BLE buffers and handled CCC properly to improve notification reliability.
- Tested with nRF Connect (mobile): device discovery, connection, and notification reception working.

## Week 7-8 - Threads, Semaphores and Mutex

- Reorganized the application into several Zephyr threads: initialization, acquisition, UI, BLE, and RTC.
- Added semaphores to synchronize startup, acquisition timing, BLE updates, and wake-up events between tasks.
- Introduced mutex protection for shared data structures such as the latest sampled sensor values and BLE state.
- Outcome: the software architecture became more modular, safer for concurrent access, and easier to debug.

## Week 9-10 - Eco mode - RTC

- Implemented an eco mode based on user inactivity to reduce unnecessary processing during idle periods.
- Added a lightweight power supervisor to detect inactivity, switch the system into sleep behavior, and wake it on button or touch activity.
- Linked eco mode with acquisition and display behavior so the watch can pause heavy tasks while remaining responsive.
- Outcome: improved runtime behavior and a first low-power management strategy for the smartwatch.
- Integer : RTC extern

## Week 11-12 - UI and touch screen

- Integrated the LVGL-based graphical interface and organized the UI into reusable components and screens.
- Added navigation logic for splash, home, weather, compass, activity, and BLE status screens.
- Connected touch and button events to screen changes and user activity detection.
- Outcome: the prototype now provides a complete visual interface with real-time sensor and BLE feedback on the display.
