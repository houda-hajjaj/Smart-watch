# ZSWatch – BLE Smartwatch on nRF5340 / Zephyr RTOS

A fully functional smartwatch firmware developed as part of an **Embedded Systems** course. The project runs on the **Nordic nRF5340 DK** using the **Zephyr RTOS** (nRF Connect SDK v3.1.0) and integrates multiple environmental and motion sensors, real-time signal processing, Bluetooth Low Energy communication, and a graphical user interface built with LVGL.

---

## Table of Contents

- [Overview](#overview)
- [Repository Branches](#repository-branches)
- [Hardware Overview](#hardware-overview)
- [Software Architecture](#software-architecture)
- [Features](#features)
  - [Core System](#core-system)
  - [Sensor Processing](#sensor-processing)
  - [Bluetooth Low Energy](#bluetooth-low-energy)
  - [Power Management](#power-management)
  - [User Interface](#user-interface)
- [Project Structure](#project-structure)
- [Build & Flash Instructions](#build--flash-instructions)
- [Development Timeline](#development-timeline)
- [Testing & Validation](#testing--validation)
- [Future Improvements](#future-improvements)
- [Team & Contributions](#team--contributions)
- [Supervisors](#supervisors)
- [License](#license)

---

# Overview

ZSWatch is an embedded smartwatch application designed around the **Nordic nRF5340 DK** platform. It demonstrates the integration of:

- Real-time operating systems (Zephyr RTOS)
- Multi-threaded embedded software
- Environmental and inertial sensors
- Bluetooth Low Energy (BLE)
- Sensor fusion
- Power management
- Embedded graphical user interfaces

The firmware continuously acquires sensor data, processes it in real time, and displays useful information while simultaneously exposing measurements over Bluetooth using both standard and custom GATT services.

---

# Repository Branches

| Branch | Description |
|---------|-------------|
| **main** | Stable version containing the complete smartwatch firmware. |
| **Timer_X** | Experimental branch exploring a Model-View-Controller (MVC) architecture. |

> **Note:** The `main` branch contains the fully working implementation. The `Timer_X` branch is currently a work in progress focused on improving software organization.

---

# Hardware Overview

## Development Board

- Nordic **nRF5340 DK**
- Dual-core ARM Cortex-M33
- Bluetooth Low Energy 5.x

## Sensor Expansion Board

**X-NUCLEO-IKS01A3**

Integrated sensors:

- LSM6DSO
  - 3-axis accelerometer
  - 3-axis gyroscope

- LIS2MDL
  - 3-axis magnetometer

- HTS221
  - Temperature sensor
  - Humidity sensor

- LPS22HH
  - Barometric pressure sensor

## Display

- Adafruit 2.8" TFT Touchscreen
- ILI9341 LCD Controller

## Real-Time Clock

- RV-8263-C8
- I²C Interface

---

# Software Architecture

The application follows a modular architecture based on multiple Zephyr threads.

```text
                 +-----------------------+
                 |     Sensor Drivers    |
                 +----------+------------+
                            |
                            v
                  Acquisition Thread
                            |
                            v
                 Processing Algorithms
                            |
         +------------------+------------------+
         |                                     |
         v                                     v
      LVGL UI                          BLE Services
         |                                     |
         +------------------+------------------+
                            |
                            v
                     Shared Data Buffer
                  (Mutex Protected Access)
```

Synchronization is achieved through:

- Semaphores
- Mutexes
- Thread-safe shared structures

---

# Features

## Core System

- Zephyr RTOS based application
- Deterministic **100 ms** processing cycle
- Multi-threaded design
- Modular source organization
- Thread synchronization using:
  - Semaphores
  - Mutexes

Main threads:

- Sensor acquisition
- Data processing
- BLE communication
- User interface
- RTC management
- Power management

---

## Sensor Processing

### Step Counter

- State-machine implementation
- Adaptive acceleration thresholds
- False positive filtering

### Distance Estimation

Computed from:

- Step count
- Estimated stride length

### Calories Burned

Estimated using:

- User activity
- Distance traveled
- Activity-dependent coefficients

### Barometric Altimeter

Pressure-to-altitude conversion using the international barometric formula.

### Floor Counter

Detects floor changes based on altitude variation.

### Digital Compass

Computes heading from magnetometer measurements and converts it into:

- North
- North-East
- East
- South-East
- South
- South-West
- West
- North-West

### 3D Orientation

Uses the **Madgwick Sensor Fusion Filter** combining:

- Accelerometer
- Gyroscope
- Magnetometer

Outputs:

- Roll
- Pitch
- Yaw

### Fall Detection

Based on:

- Impact acceleration threshold
- Post-impact inactivity period

### Activity Recognition

Automatically classifies:

- Rest
- Walking
- Running

Using acceleration magnitude.

### Weather Trend Analysis

Pressure history is processed using linear regression to estimate:

- Improving weather
- Stable weather
- Worsening weather

---

## Bluetooth Low Energy

### Standard GATT Service

Environmental Sensing Service

| Characteristic | UUID |
|---------------|------|
| Temperature | 0x2A6E |
| Humidity | 0x2A6F |
| Pressure | 0x2A6D |

### Custom Motion Service

UUID

```
12345678-1234-5678-1234-56789abc0000
```

Characteristics:

| Data | UUID Suffix |
|------|-------------|
| Accelerometer | 0004 |
| Magnetometer | 0005 |
| Step Counter | 0006 |

Features:

- Notifications
- CCC handling
- BLE advertising
- Connection management

Advertising name:

```
Smart Watch Sensor Hub
```

The Bluetooth controller executes on the network core using:

```
hci_ipc
```

---

## Power Management

Power-saving features include:

- Automatic sleep after **30 seconds** of inactivity
- Suspension of non-essential threads
- Motion-triggered wake-up
- Wake-up on:
  - Step detection
  - Fall detection
- BLE status LED

---

## User Interface

The graphical interface is built using **LVGL**.

Available screens include:

- Home
- Weather
- Compass
- Activity
- BLE Status

Features:

- Real-time updates
- Touchscreen interaction
- Physical button support
- Responsive navigation

---

# Project Structure

```text
Smart_Watch/
│
├── src/
│   ├── ble/
│   ├── sensors/
│   ├── processing/
│   ├── ui/
│   ├── rtc/
│   ├── power/
│   └── main.c
│
├── include/
│
├── boards/
│
├── CMakeLists.txt
│
├── prj.conf
│
└── README.md
```

---

# Build & Flash Instructions

## Prerequisites

Install:

- nRF Connect SDK v3.1.0
- Zephyr RTOS
- West
- CMake
- Ninja
- ARM Toolchain

---

## Clone Repository

```bash
git clone https://github.com/NguyenPhuong242/Smart_Watch.git

cd Smart_Watch
```

---

## Build Application

```bash
west build -b nrf5340dk/nrf5340/cpuapp -- \
-DSHIELD="x_nucleo_iks01a3;adafruit_2_8_tft_touch_v2"
```

---

## Flash Network Core

Build the Bluetooth controller once.

```bash
west build -b nrf5340dk/nrf5340/cpunet \
zephyr/samples/bluetooth/hci_ipc
```

Flash:

```bash
west flash
```

---

## Flash Application Core

```bash
west flash
```

---

# Development Timeline

| Weeks | Work Completed |
|--------|----------------|
| 1–2 | Environment setup, SDK configuration, first successful build |
| 3–4 | Sensor integration and calibration |
| 5–6 | BLE GATT services and notifications |
| 7–8 | Multithreading, semaphores and mutex synchronization |
| 9–10 | RTC integration and power-saving mode |
| 11–12 | LVGL interface, touchscreen and navigation |

---

# Testing & Validation

The firmware was validated through functional testing.

✔ Sensor communication over I²C

✔ Stable BLE advertising

✔ Reliable BLE connections

✔ GATT notifications every 2 seconds

✔ Correct acquisition of:

- Temperature
- Humidity
- Pressure
- Acceleration
- Magnetometer

✔ Correct UI refresh

✔ Touchscreen responsiveness

✔ Automatic transition to low-power mode

✔ Wake-up triggered by movement

---

# Future Improvements

Planned enhancements include:

- Automatic magnetometer calibration
- TinyML-based adaptive activity recognition
- Enhanced BLE offloading using the network core
- Flash-based activity history logging
- Complete LVGL dashboard
- Heart-rate monitoring using an external PPG sensor
- Battery monitoring
- OTA firmware updates

---

# Team & Contributions

| Team Member | Main Contributions |
|--------------|-------------------|
| **Houda HAJJAJ** | BLE stack, shared data management, GATT services |
| **Rim EL KILI** | Signal processing, sensor fusion, embedded algorithms |
| **Khanh-Phuong NGUYEN** | Sensor drivers, data acquisition, RTC |
| **Deraniaina Rafelimanana** | System integration, LVGL interface, project coordination |

---

# Supervisors

- Noël RICHARD
- Hervé BOEGLEN

---

# License

This project was developed for educational purposes as part of an Embedded Systems course.

Feel free to fork, study, and extend the project while properly crediting the original authors.

---


---

## Acknowledgments

Special thanks to our supervisors for their guidance throughout the project and to the Zephyr and Nordic developer communities for their extensive documentation and open-source ecosystem.
````
