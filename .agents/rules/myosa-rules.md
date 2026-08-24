# Project Rules & Customizations for MYOSA ESP32 Multi-Sensor Platform (Sky-Mast)

## Hardware Mapping & I2C Address Register Map

| Subsystem / Sensor | Chip Model | I2C Address | Crucial Setup / Quirks & Fixes |
| :--- | :---: | :---: | :--- |
| **Microcontroller** | ESP32-D0WD-V3 | — | Target FQBN: `esp32:esp32:esp32` at 115200 baud serial profile |
| **OLED Display** | SSD1306 | `0x3C` | 128x64 Mono OLED (5 Pages + Mini Artificial Horizon Box) |
| **IMU Motion** | MPU6050 | `0x68` | Scaled at $16384\,\text{LSB}/g$; Pitch, Roll, and >45° High Tilt alarm |
| **Barometer** | BMP180 | `0x77` | Measures Air Pressure ($\text{hPa}$), Temp ($^\circ\text{C}$), Altitude ($\text{m}$) |
| **Actuator Expander**| PCA9536 | `0x41` | Drives Relay (`AC_SWITCH_IO`) & Active DC Buzzer (`BUZZER_IO`) |
| **Gesture & Light** | APDS9960 | `0x39` | **Chip ID `0x9E` Variant** (Requires library chip ID patch) |

---

## APDS9960 Gesture Sensor Hardware Lessons & Known Quirks

1. **Hardware Variant Chip ID (`0x9E`)**:
   - Some APDS9960 hardware modules return chip ID `0x9E` on register `0x92` instead of standard `0xAB`, `0x9C`, or `0xA8`.
   - Always verify that [`LightProximityAndGesture.h`](file:///home/sreyas/Arduino/libraries/LightProximityAndGesture/src/LightProximityAndGesture.h) and [`LightProximityAndGesture.cpp`](file:///home/sreyas/Arduino/libraries/LightProximityAndGesture/src/LightProximityAndGesture.cpp) include `0x9E` in accepted device ID checks.

2. **FIFO Reader Decoding**:
   - `readGesture()` requires proper decoding of accumulated directional FIFO data (`_gesture_data`) rather than returning `TIMEOUT`.
   - Configure `GPULSE` (`0xA6`) for 8 pulses at 16µs and set `GCONF2` (`0xA3`) for 300% IR LED boost drive (`0x60`).
   - `PGAIN_8X` and `GGAIN_8X` maximum analog gain should be set to assist low-reflection photodiode modules.

3. **Auto-Cycle Idle Fallback**:
   - If the APDS9960 IR sensor is disconnected or has low reflection, the software automatically advances OLED dashboard screens every 6 seconds.

---

## Build & Upload Commands

- **Compile**:
  ```bash
  /home/sreyas/.local/bin/arduino-cli compile --fqbn esp32:esp32:esp32 /home/sreyas/myosa-accel-demo
  ```
- **Upload**:
  ```bash
  /home/sreyas/.local/bin/arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 /home/sreyas/myosa-accel-demo
  ```
- **Monitor Serial Output**:
  ```bash
  python3 -c "import serial, time; s=serial.Serial('/dev/ttyUSB0', 115200, timeout=1); ..."
  ```
