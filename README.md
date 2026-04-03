# ZSWatch BLE Sensor Project

This project implements a Bluetooth Low Energy (BLE) smartwatch prototype using an **nRF5340 DK** and an **ST IKS01A3 shield**. It runs on **Zephyr RTOS** (nRF Connect SDK v3.1.0) and demonstrates periodic sensor data transmission over BLE using standard GATT services.

##  Features
- **Hardware**: nRF5340 DK (application core) + X-NUCLEO-IKS01A3 shield (LSM6DSO, LIS2MDL, HTS221, LPS22HH).
- **Sensor drivers**: Periodic reading of temperature, humidity, pressure, acceleration, and magnetic field.
- **BLE implementation**:
  - Environmental Sensing Service (ESS, UUID 0x181A) with standard characteristics:
    - Temperature (0x2A6E)
    - Humidity (0x2A6F)
    - Pressure (0x2A6D)
    - 3‑axis accelerometer (0x2AA1)
    - 3‑axis magnetometer (0x2AA0)
  - Notifications enabled for all characteristics.
  - Device name: "ZSWatch".
- **Robust communication**: Increased BLE buffers, proper CCC handling, and dual‑core setup (network core runs `hci_ipc`).


````
Arguments Cmake:

-DSHIELD=x_nucleo_iks01a3, adafruit_2_8_tft
````

##  Status
- Sensors initialise correctly (I²C).
- BLE advertising and connection functional.
- Notifications sent every 2 seconds with scaled sensor values.
- Tested with nRF Connect for Mobile – data appears in real‑time after subscribing.

# ZSWatch Project Log

This is a short log of the first three weeks of development.

## Week 1-2 — Initialize project and configuration

- Created the project structure and initialized the repository.
- Set up the Zephyr / nRF Connect SDK environment; added `CMakeLists.txt` and `prj.conf` for the `nrf5340dk` target.
- Performed an initial build with `west build` to verify the toolchain and resolve dependencies.
- Outcome: initial build succeeded and base configuration validated.

## Week 3-4 — Finish sensors and USART

- Integrated sensor drivers (altimeter, compass, mag_sensor, env_sensor, motion_sensor, step_counter) and configured I²C/SPI.
- Implemented periodic read routines and unit conversions for temperature, pressure, humidity, acceleration and magnetism.
- Added a USART debug channel to log readings and simplify hardware debugging.
- Hardware tests: stable readings obtained and initial calibrations applied.

## Week 5-6 — BLE

- Implemented main BLE services (Environmental Sensing Service and related characteristics).
- Enabled notifications to periodically send measurements to a connected client.
- Tuned BLE buffers and handled CCC properly to improve notification reliability.
- Tested with nRF Connect (mobile): device discovery, connection and notification reception working.

## Week 7-8 - Threads, Semaphores and Mutex

## Week 9-10 - Mode Eco

## Week 11-12 - UI and Touch screen

Updated file: [README.md](README.md)
