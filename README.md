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

##  Status
- Sensors initialise correctly (I²C).
- BLE advertising and connection functional.
- Notifications sent every 2 seconds with scaled sensor values.
- Tested with nRF Connect for Mobile – data appears in real‑time after subscribing.

##  Quick Test
1. Build and flash the application:
   ```bash
   west build -b nrf5340dk/nrf5340/cpuapp --pristine
   west flash

## Next Steps
- Integrate display (LVGL)
- Add RTC for calendar/time features
- Implement custom service for combined data
