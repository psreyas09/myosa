# Sky-Mast - Handheld Smart Environmental & Orientation Monitor
**MYOSA ESP32 Multi-Sensor Platform System Documentation**

---

## 📖 Overview

**Sky-Mast** is an advanced environmental and 3D orientation monitoring system built for the **MYOSA (Make Your Own Sensors Applications)** modular hardware kit on the **ESP32** platform. It provides real-time sensor processing, interactive OLED dashboard rendering with an artificial horizon, fail-safe hazard alerts, touchless gesture navigation, and full **MYOSA Android Mobile App** wireless connectivity over Bluetooth SPP.

---

## 🛠️ Hardware Architecture & Sensor Map

| Hardware Module | Sensor Chip | I2C Address | Function / Features |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | ESP32-D0WD-V3 | — | Dual-core 240MHz MCU with Bluetooth Classic & WiFi |
| **OLED Display** | SSD1306 | `0x3C` | 128x64 Mono OLED (5 Dashboard Pages + Artificial Horizon) |
| **IMU Motion** | MPU6050 | `0x68` / `0x69` | 3-Axis Accel & Gyro ($16384\,\text{LSB}/g$ scaling, Pitch/Roll/Tilt) |
| **Barometer** | BMP180 / BMP085 | `0x77` | Air Pressure ($\text{hPa}$), Altitude ($\text{m}$), Temperature ($\text{^\circ C}$) |
| **Gesture & Light** | APDS9960 | `0x39` | Ambient Light, Proximity, 6-Directional Swipes (Chip ID `0x9E` variant patched in library) |
| **Actuator Expansion** | PCA9536 | `0x41` | 4-Pin I2C IO Expander driving Relay (`AC_SWITCH_IO`) & Active Buzzer (`BUZZER_IO`) |
| **Buzzer Pins** | Active Buzzer | GPIO D4, D12, D13, D27 | Multi-pin fail-safe active DC buzzer drive |

---

## 📁 Software Architecture & Code Directory

```
/home/sreyas/myosa-accel-demo/
├── myosa-accel-demo.ino   # Main sketch entry point (calls setup & loop)
├── main.cpp                # Application setup & non-blocking execution loop
├── utils.h / utils.cpp     # System state containers, non-blocking timers, math helpers
├── display.h / display.cpp # OLED dashboard renderer (5 pages, warning overlay, horizon)
├── imu.h / imu.cpp         # MPU6050 acceleration scaling, tilt calculation, motion detect
├── barometer.h / barometer.cpp # BMP180 pressure, temperature, altitude rate calculation
├── gesture.h / gesture.cpp # APDS9960 gesture engine, I2C 0x39 diagnostic probe
├── alerts.h / alerts.cpp   # Fail-safe active buzzer control, >45° tilt alarm, command parser
└── bt.h / bt.cpp           # MYOSA Mobile App Bluetooth SPP protocol & telemetry engine
```

---

## 🖥️ OLED Dashboard Navigation (5 Pages)

| Page | Dashboard Screen | Key Metrics Displayed |
| :---: | :--- | :--- |
| **P1** | **Environment** | Temperature ($\text{^\circ C}$), Air Pressure ($\text{hPa}$), Altitude ($\text{m}$), Elevation Rate ($\text{m/s}$) |
| **P2** | **Orientation** | Pitch ($^\circ$), Roll ($^\circ$), Tilt X/Y/Z ($^\circ$), Live Mini Artificial Horizon Box |
| **P3** | **Motion Data** | Accel X/Y/Z ($g$), Gyro X/Y/Z ($^\circ/\text{s}$), Motion Detection Status |
| **P4** | **System Status** | Motion Detect (YES/NO), Bluetooth (READY), Sensor Health (ALL OK), Active Alerts |
| **P5** | **Gesture Monitor** | APDS9960 Hardware Status (`0x39`), Proximity Live Bar Graph (0-255), Last Gesture |

---

## 📱 MYOSA Mobile App Protocol Integration

### 1. Bluetooth Connection
- **Device Broadcast Name**: **`MYOSA_1`**
- **Protocol**: Bluetooth Classic SPP (Serial Port Profile at 115200 baud).

### 2. Telemetry Packet Stream (`send_data()`)
Streams a **33-value comma-separated string** every 100 ms with CRLF (`\r\n`) line endings:
```csv
*,*,*,*,*,23.50,19.05,0.03,2540.00,*,*,*,*,*,*,*,*,*,0.02,-0.01,0.98,0.10,-0.05,0.02,1.20,-0.80,1.44,23.50,77.72,50.00,23.50\r\n
```

### 3. Actuator Control Commands
Control commands received from the MYOSA Mobile App (via Bluetooth or Wi-Fi with prefix `'w'`):

| Command | Action | Technical Detail |
| :---: | :--- | :--- |
| **`r`** / **`wr`** | **Relay ON for 30 Seconds** | Activates `AC_SWITCH_IO` (Relay) with 30,000 ms auto-off timer |
| **`c`** / **`wc`** | **Relay OFF** | Immediately deactivates `AC_SWITCH_IO` and clears timer |
| **`b`** / **`wb`** | **Buzz 5 Times** | Triggers 5 sequential beep pulses (120 ms interval) |
| **`1`** / **`w1`** | **Output Pin 1 HIGH** | Sets PCA9536 `IO2` (User Pin 1) to `HIGH` |
| **`2`** / **`w2`** | **Output Pin 2 HIGH** | Sets PCA9536 `IO3` (User Pin 2) to `HIGH` |
| **`3`** / **`w3`** | **Output Pin 1 LOW** | Sets PCA9536 `IO2` (User Pin 1) to `LOW` |
| **`4`** / **`w4`** | **Output Pin 2 LOW** | Sets PCA9536 `IO3` (User Pin 2) to `LOW` |

---

## ⚡ Buzzer & Hazard Warning Logic

- **Normal Level Operation (Tilt $\le$ 45°)**: Buzzer pins (D4, D12, D13, D27 & `BUZZER_IO`) driven **0V LOW (100% Silence)**.
- **High Tilt Alarm (Tilt > 45°)**: Flashes `WARNING: HIGH TILT > 45 deg` overlay on OLED and drives buzzer **5V HIGH in 150ms pulses**.
- **Startup / Interaction**: Plays a 150ms startup beep on boot, and 80ms confirmation beeps on gesture page switches.

---

## 🚀 Building & Flashing via `arduino-cli`

### 1. Compile Sketch
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 /home/sreyas/myosa-accel-demo
```

### 2. Upload to Board
```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 /home/sreyas/myosa-accel-demo
```

### 3. Monitor Serial Output
```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

---

## 🔍 Hardware Troubleshooting & Multimeter Guide

### 1. Power Test (VCC)
- **Multimeter Mode**: DC Voltage ($V\overline{\dots}$ 20V range)
- **Probes**: Black on **GND**, Red on **VCC**
- **Expected Reading**: **`+3.3V DC`** ($\pm 0.1\text{V}$)

### 2. I2C Bus Test (SDA & SCL)
- **Multimeter Mode**: DC Voltage ($V\overline{\dots}$)
- **Probes**: Black on **GND**, Red on **SDA** / **SCL**
- **Expected Reading**: **`2.7V to 2.9V DC`** (Indicates active 100 kHz I2C clock/data pulses)

### 3. Smartphone Camera IR Emitter Test
- Open smartphone camera app (use **front selfie camera** without IR filter).
- Point camera at the black APDS9960 sensor chip while powered on.
- **Working IR LED**: Visible **faint purple/pink glowing light** inside the lens.
- **Defective IR LED**: Completely dark under camera (IR emitter burnt out).
