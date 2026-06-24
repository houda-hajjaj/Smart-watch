# ZSWatch – BLE Smartwatch on nRF5340 / Zephyr RTOS

This repository contains the firmware for a fully functional smartwatch prototype developed as part of an embedded systems course. The project runs on the **Nordic nRF5340 DK** and leverages the **Zephyr RTOS** (nRF Connect SDK v3.1.0). It integrates environmental and motion sensors, provides advanced on‑device signal processing, and exposes sensor data via Bluetooth Low Energy using standard and custom GATT services.

---

## Repository Branches

| Branch      | Description                                                                                          |
|-------------|------------------------------------------------------------------------------------------------------|
| `main`      | Main application code (sensors, BLE, UI). Modular design for readability and maintainability.        |
| `Timer_X`   | Experimental refactoring toward a **Model‑View‑Controller (MVC)** architecture.                      |

> The `main` branch is the stable version; `Timer_X` is a work in progress exploring clearer separation of data acquisition (Model), user interface (View), and application logic (Controller).

---

## Hardware Overview

- **MCU**: nRF5340 DK (dual‑core Cortex‑M33)  
- **Sensor Shield**: X‑NUCLEO‑IKS01A3  
  - LSM6DSO (accelerometer + gyroscope)  
  - LIS2MDL (magnetometer)  
  - HTS221 (temperature + humidity)  
  - LPS22HH (barometric pressure)  
- **Display**: Adafruit 2.8″ TFT touchscreen (ILI9341)  
- **RTC**: External RV‑8263‑C8 (I²C)  

---

## Software Features

### Core System
- Multi‑threaded real‑time pipeline with deterministic 100 ms cycle  
- Threads: acquisition, processing, BLE, UI, RTC, power management  
- Semaphore‑based synchronization and mutex‑protected shared data  

### Sensor Processing
- **Step counter** (state‑machine, adaptive thresholds)  
- **Distance & calories** (stride length, activity‑dependent coefficients)  
- **Barometric altimeter** (pressure‑to‑altitude conversion with reference setting)  
- **Floor counter** (altitude change detection)  
- **Digital compass** (heading from magnetometer, cardinal direction conversion)  
- **3D orientation** (Madgwick filter fusion of accelerometer, gyroscope, magnetometer)  
- **Fall detection** (impact threshold + post‑impact inactivity)  
- **Activity classification** (rest, walking, running based on acceleration magnitude)  
- **Weather trend analysis** (linear regression on pressure history)  

### Bluetooth Low Energy (BLE)
- **Standard GATT service** – Environmental Sensing (UUID 0x181A)  
  - Temperature (0x2A6E), Humidity (0x2A6F), Pressure (0x2A6D)  
- **Custom GATT service** – Motion (UUID 12345678‑1234‑5678‑1234‑56789abc0000)  
  - Accelerometer (0004), Magnetometer (0005), Step counter (0006)  
- **Notifications** per characteristic with CCC handling  
- **Advertising** name: `Smart Watch Sensor Hub`  
- Dual‑core architecture: network core runs `hci_ipc` for BLE controller  

### Power Management
- Inactivity timer (30 s) triggers sleep mode (thread suspension)  
- Motion‑triggered wake‑up (step or fall detection)  
- LED indicator for BLE connection state  

### User Interface
- **LVGL** – graphical UI with screens: home, weather, compass, activity, BLE status  
- Touch and button event handling  
- Real‑time update of sensor and system data on display  

---

## Build & Flash Instructions

### Prerequisites
- nRF Connect SDK v3.1.0 (or compatible Zephyr version)  
- West tool, CMake, Ninja, and a suitable ARM toolchain  

### Setup
Clone the repository and initialize the environment (if not already done):

```bash
git clone https://github.com/houda-hajjaj/Smart-watch.git
cd Smart_Watch
