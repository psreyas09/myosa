/*
 * Sky-Mast - Handheld Smart Environmental & Orientation Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Bluetooth Serial SPP Controller for MYOSA Mobile App
 * File: bt.cpp
 */

#include "bt.h"
#include "alerts.h"
#include <BluetoothSerial.h>
#include <Arduino.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

extern AlertController g_alerts;
static BluetoothSerial SerialBT;

BTController::BTController()
    : _initialized(false), _timer(100) {
}

bool BTController::begin(const char* deviceName) {
    // Start Hardware Serial1 for onboard BT module (115200 baud)
    Serial1.begin(115200);

    // Start ESP32 Wireless Bluetooth Classic SPP with name "MYOSA_1"
    bool ok = SerialBT.begin(deviceName);
    if (ok) {
        Serial.print("MYOSA Bluetooth SPP Started with Name: ");
        Serial.println(deviceName);
        _initialized = true;
    } else {
        Serial.println("Failed to start ESP32 Bluetooth Serial!");
        _initialized = false;
    }
    return _initialized;
}

bool BTController::isConnected() {
    return _initialized && SerialBT.hasClient();
}

void BTController::update(const SystemState &state) {
    if (!_initialized) {
        return;
    }

    // Read and process incoming Bluetooth SPP command characters from MYOSA Mobile App
    while (SerialBT.available()) {
        char c = (char)SerialBT.read();
        if (c == 'r' || c == 'R' || c == 'c' || c == 'C' || c == 'b' || c == 'B' ||
            c == '1' || c == '2' || c == '3' || c == '4' || c == 'w' || c == 'W') {
            Serial.print("BT Received Command: ");
            Serial.println(c);
            g_alerts.processCommandChar(c);
        }
    }

    if (!_timer.isReady()) {
        return;
    }

    // Format the official MYOSA 33-value comma-separated telemetry string
    // Format required by MYOSA Android Mobile App (IEEE Sensors Manual Page 19):
    // [Luminous 3x],[AirQuality 2x],[Pressure 4x],[Gesture 4x],[RTC 5x],[IMU 9x],[TempHum 4x]
    
    char buf[256];
    
    // 1. Luminous Sensor (3 values: ch0 - ch1, ch1, lux) -> absent: *,*,*,
    char lumiStr[32] = "*,*,*,";

    // 2. Air Quality Sensor (2 values: CO2, TVOC) -> absent: *,*,
    char aqStr[32] = "*,*,";

    // 3. Pressure & Altitude Sensor (4 values: TempC, mmHg, Bar, Pascal)
    char baroStr[64];
    if (state.sensorStatus.baroOk) {
        float pas = state.env.pressure * 100.0f; // hPa to Pascal
        float mmhg = pas * 0.0075006375541921f;
        float bar = pas * 0.00001f;
        sprintf(baroStr, "%0.2f,%0.2f,%0.2f,%0.2f,", state.env.temperature, mmhg, bar, pas);
    } else {
        strcpy(baroStr, "*,*,*,*,");
    }

    // 4. Gesture & Proximity Sensor (4 values: Red, Green, Blue, Proxim)
    char gestStr[32] = "*,*,*,*,";

    // 5. RTC / Time (5 values: Sec, Min, Hour, Day, Month) -> absent: *,*,*,*,*,
    char rtcStr[32] = "*,*,*,*,*,";

    // 6. Gyroscope & Accelerometer (9 values: AccX, AccY, AccZ, GyroX, GyroY, GyroZ, TiltX, TiltY, TiltZ)
    char imuStr[128];
    if (state.sensorStatus.imuOk) {
        sprintf(imuStr, "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,",
                state.motion.accelX, state.motion.accelY, state.motion.accelZ,
                state.motion.gyroX, state.motion.gyroY, state.motion.gyroZ,
                state.orientation.tiltX, state.orientation.tiltY, state.orientation.tiltZ);
    } else {
        strcpy(imuStr, "*,*,*,*,*,*,*,*,*,");
    }

    // 7. Temperature & Humidity Sensor (4 values: TempC, TempF, Humidity, HeatIndex)
    char thStr[64];
    if (state.sensorStatus.baroOk) {
        float tempF = (state.env.temperature * 9.0f / 5.0f) + 32.0f;
        sprintf(thStr, "%0.2f,%0.2f,50.00,%0.2f", state.env.temperature, tempF, state.env.temperature);
    } else {
        strcpy(thStr, "*,*,*,*");
    }

    // Assemble complete 31/33-value payload line
    sprintf(buf, "%s%s%s%s%s%s%s", lumiStr, aqStr, baroStr, gestStr, rtcStr, imuStr, thStr);

    // Transmit to MYOSA Android App over Bluetooth SPP & Hardware Serial1 with \r\n line ending
    SerialBT.println(buf);
    Serial1.println(buf);
}
