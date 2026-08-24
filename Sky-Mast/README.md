# Sky-Mast: Handheld Smart Environmental & Orientation Monitor

**Sky-Mast** is a handheld environmental, motion, and orientation monitoring system built specifically for the **MYOSA ESP32 Controller Board** by MakeSense EduTech.

---

## Key Features

- **Multi-Sensor Data Fusion**: Continuous monitoring of Altitude, Atmospheric Pressure, Ambient Temperature, Pitch, Roll, 3-Axis Acceleration, 3-Axis Gyroscope, and Motion status.
- **Strict MYOSA Library Integration**: Built using standard MYOSA hardware libraries (`MYOSA`, `OLED`, `AccelAndGyro`, `BarometricPressure`, `LightProximityAndGesture`, `Actuator`), avoiding third-party Adafruit driver dependencies.
- **Gesture-Driven Dashboard**: Touchless screen navigation using APDS9960 gesture sensor (Swipe Right for Next Page, Swipe Left for Previous Page).
- **Non-Blocking Architecture**: Fully event-driven using `millis()` timing with specific refresh rates for each sensor and display module. No blocking `delay()` calls in the main application loop.
- **Fault-Tolerant Startup**: Gracefully detects missing or disconnected sensors at boot and displays `Sensor Error` instead of crashing.
- **Audio & Visual Alerts**:
  - **High Tilt Alert**: Triggers flashing OLED overlay, blinking Actuator LED, and active Buzzer beep when tilt angle exceeds 45°.
  - **Rapid Elevation Alert**: Displays an on-screen warning banner when altitude changes rapidly (>1.5 m/s).
- **Modular C++ Design**: Cleanly separated into modular `.h` and `.cpp` files for easy maintenance and future extension.

---

## Hardware Architecture & Module Wiring

### Hardware Available
1. **MYOSA ESP32 Controller Board** (ESP32 Dev Module)
2. **OLED Display (SSD1306)** - 128x64 pixels, I2C address `0x3C`
3. **MPU6050 Accelerometer + Gyroscope** - 6-axis IMU, I2C address `0x69`
4. **BMP180 Barometric Pressure Sensor** - Altitude & Pressure, I2C address `0x77`
5. **APDS9960 Light, Proximity & Gesture Sensor** - I2C address `0x39`
6. **Actuator Board (PCA9536)** - 4-bit I2C IO Expander driving Buzzer (`IO1`) and LED/Relay (`IO0`), address `0x41`

### Bus Interface & Wiring Assumptions
All MYOSA sensor cards connect to the MYOSA Controller motherboard via the standardized I2C bus:
- **I2C SDA**: ESP32 GPIO 21 (Default Wire pin)
- **I2C SCL**: ESP32 GPIO 22 (Default Wire pin)
- **Bus Speed**: 100 kHz standard mode

---

## Software Architecture & Directory Structure

```
Sky-Mast/
├── Sky-Mast.ino      # Main sketch entry point (Arduino CLI / IDE compatibility)
├── main.cpp          # System initialization, boot sequence, and non-blocking main loop
├── utils.h           # System state structures, non-blocking timer class, & math helpers
├── utils.cpp         # Implementation of timer, pitch/roll calculation, & float formatters
├── imu.h             # MPU6050 Accelerometer & Gyroscope controller header
├── imu.cpp           # Non-blocking 100 ms IMU sampling & pitch/roll estimation
├── barometer.h       # BMP180 Barometric Pressure & Altitude sensor header
├── barometer.cpp     # Non-blocking 500 ms Barometer sampling & elevation rate monitor
├── gesture.h         # APDS9960 Gesture sensor header
├── gesture.cpp       # Non-blocking 50 ms gesture detection & page navigation handler
├── alerts.h          # Actuator PCA9536 LED & Buzzer controller header
├── alerts.cpp        # Tilt >45° alert evaluation, LED flash, & buzzer beep driver
├── display.h         # OLED dashboard renderer header
├── display.cpp        # OLED pages (Env, Orientation, Motion, System) & warning overlays
└── README.md         # Comprehensive project documentation
```

---

## OLED Dashboard Layouts

### Header Bar
- App Title: `SKY-MAST`
- Page Indicator: `[P1/4]`
- Animated Activity Spinner: `| / - \`

### Screen 1: Environment (`[P1/4]`)
- **Altitude**: Measured in meters (m)
- **Pressure**: Measured in Hectopascals (hPa)
- **Temperature**: Measured in °C
- **P-Bar**: Dynamic horizontal progress bar showing atmospheric pressure relative to standard sea level.

### Screen 2: Orientation (`[P2/4]`)
- **Pitch**: Calculated pitch angle (-180° to 180°)
- **Roll**: Calculated roll angle (-180° to 180°)
- **Tilt X / Y / Z**: Raw tilt angles in degrees
- **Horizon Bar**: Visual artificial horizon box.

### Screen 3: Motion (`[P3/4]`)
- **AccX, AccY, AccZ**: 3-axis acceleration in g
- **GyrX, GyrY, GyrZ**: 3-axis angular velocity in °/s

### Screen 4: System (`[P4/4]`)
- **Motion Det**: `YES` / `NO` (MPU6050 motion engine status)
- **Bluetooth**: `READY`
- **Sensors**: `ALL OK` or degraded sensor report
- **Alerts**: Active alert summary

---

## Sensor Refresh Rates & Timing Configuration

| Sensor / Module | Refresh Interval | Method |
| :--- | :--- | :--- |
| **APDS9960 Gesture** | `50 ms` | Non-blocking `millis()` timer |
| **MPU6050 IMU** | `100 ms` | Non-blocking `millis()` timer |
| **OLED Display** | `100 ms` | Non-blocking `millis()` timer |
| **BMP180 Barometer** | `500 ms` | Non-blocking `millis()` timer |

---

## Build Instructions

### Prerequisites
1. **Arduino CLI** or **Arduino IDE 2.x** installed.
2. **ESP32 Core** installed (`esp32:esp32` package).
3. **MYOSA Libraries** present in your Arduino libraries path (`~/Arduino/libraries/`):
   - `MYOSA`
   - `OLED`
   - `AccelAndGyro`
   - `BarometricPressure`
   - `LightProximityAndGesture`
   - `Actuator`

### Building with Arduino CLI

To compile `Sky-Mast` for ESP32 Dev Module:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 /path/to/Sky-Mast
```

To upload to an ESP32 board connected on `/dev/ttyUSB0`:

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 /path/to/Sky-Mast
```

---

## Memory Optimization & Performance

- **Flash Footprint**: ~349 KB (~26% of standard 1.3 MB partition).
- **RAM Footprint**: ~24 KB (~7% of 320 KB RAM), leaving over 300 KB for stack, heap, and future connectivity tasks.
- **Pass-By-Reference**: Data structures (`SystemState`) are passed by reference (`SystemState &state`) to eliminate buffer allocations in tight loop iterations.
- **No Blocking Delays**: Sensor polling routines utilize light lightweight state checks, keeping the main loop execution time below 2 ms per cycle.

---

## Future Expansion Architecture

The codebase contains predefined data hooks in `utils.h` (`ExpansionModules` struct) and modular drivers to seamlessly plug in future hardware:

1. **GPS Module**: Add `gps.h` / `gps.cpp` reading NMEA over Serial2, updating `state.expansion.gps`.
2. **Compass / Magnetometer**: Add tilt-compensated heading calculation in `imu.cpp`.
3. **BLE Connectivity**: Expose ESP32 BLE UART service to transmit `SystemState` JSON packets to mobile apps.
4. **Wi-Fi Web Dashboard**: Host a lightweight HTTP/AsyncWebServer on ESP32 displaying live telemetry graphs.
5. **Cloud Telemetry**: Send periodic HTTP POST / MQTT payloads to AWS IoT or ThingsBoard.
6. **SD Card Logging**: Append periodic CSV sensor records to an SPI SD card.
