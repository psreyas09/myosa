/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Inertial Measurement Unit (MPU6050) & Structural Vibration Engine
 * File: imu.cpp
 */

#include "imu.h"
#include "alerts.h"
#include <Wire.h>

extern AlertController g_alerts;

IMUController::IMUController()
    : _sampleTimer(10), // 100 Hz (10 ms) high-speed sampling for FFT analysis
      _fftTimer(80),    // 80 ms FFT evaluation rate
      _initialized(false) {
}

bool IMUController::begin() {
    _initialized = _ag.begin(false);
    if (!_initialized) {
        _initialized = _ag.ping();
    }
    _fft.reset();
    return _initialized;
}

bool IMUController::isConnected() {
    return _ag.ping();
}

void IMUController::resetFFT() {
    _fft.reset();
}

bool IMUController::update(SystemState &state) {
    if (!_sampleTimer.isReady()) {
        return false;
    }

    bool connected = _ag.ping();
    state.sensorStatus.imuOk = connected;

    if (!connected) {
        state.orientation.pitch = 0.0f;
        state.orientation.roll = 0.0f;
        state.orientation.tiltX = 0.0f;
        state.orientation.tiltY = 0.0f;
        state.orientation.tiltZ = 0.0f;
        state.orientation.totalDeflection = 0.0f;
        state.motion.motionDetected = false;
        state.structure.resonanceHazard = false;
        return false;
    }

    // Direct 100 Hz raw I2C burst read from MPU6050 registers (probing 0x69 and 0x68)
    uint8_t mpuAddr = 0x69;
    Wire.beginTransmission(mpuAddr);
    Wire.write(0x3B);
    if (Wire.endTransmission(false) != 0) {
        mpuAddr = 0x68;
        Wire.beginTransmission(mpuAddr);
        Wire.write(0x3B);
    }

    float ax = 0.0f, ay = 0.0f, az = 1.0f;
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;

    if (Wire.endTransmission(false) == 0 && Wire.requestFrom((uint8_t)mpuAddr, (size_t)14) == 14) {
        uint8_t xh = Wire.read(); uint8_t xl = Wire.read();
        uint8_t yh = Wire.read(); uint8_t yl = Wire.read();
        uint8_t zh = Wire.read(); uint8_t zl = Wire.read();
        Wire.read(); Wire.read(); // Temperature high/low
        uint8_t gxh = Wire.read(); uint8_t gxl = Wire.read();
        uint8_t gyh = Wire.read(); uint8_t gyl = Wire.read();
        uint8_t gzh = Wire.read(); uint8_t gzl = Wire.read();

        int16_t rawAX = (int16_t)((uint16_t)xh << 8 | xl);
        int16_t rawAY = (int16_t)((uint16_t)yh << 8 | yl);
        int16_t rawAZ = (int16_t)((uint16_t)zh << 8 | zl);

        int16_t rawGX = (int16_t)((uint16_t)gxh << 8 | gxl);
        int16_t rawGY = (int16_t)((uint16_t)gyh << 8 | gyl);
        int16_t rawGZ = (int16_t)((uint16_t)gzh << 8 | gzl);

        // Convert 16-bit raw values to acceleration in g (16384 LSB/g)
        ax = (float)rawAX / 16384.0f;
        ay = (float)rawAY / 16384.0f;
        az = (float)rawAZ / 16384.0f;

        // Convert gyro to deg/s (131 LSB/dps for +/- 250 deg/s)
        gx = (float)rawGX / 131.0f;
        gy = (float)rawGY / 131.0f;
        gz = (float)rawGZ / 131.0f;
    } else {
        ax = (float)_ag.getAccelX(false) / 16384.0f;
        ay = (float)_ag.getAccelY(false) / 16384.0f;
        az = (float)_ag.getAccelZ(false) / 16384.0f;
        gx = _ag.getGyroX(false);
        gy = _ag.getGyroY(false);
        gz = _ag.getGyroZ(false);
    }

    state.motion.accelX = ax;
    state.motion.accelY = ay;
    state.motion.accelZ = az;
    state.motion.gyroX = gx;
    state.motion.gyroY = gy;
    state.motion.gyroZ = gz;

    // Feed signed 3D dynamic sway signal into FFT buffer
    float linearSway = ax + ay + (az - 1.0f);
    _fft.addSample(linearSway);

    // Calculate Pitch & Roll in degrees (-90° to +90°)
    float txRaw = atan2f(ax, sqrtf(ay * ay + az * az)) * 180.0f / (float)M_PI;
    float tyRaw = atan2f(ay, sqrtf(ax * ax + az * az)) * 180.0f / (float)M_PI;
    float tzRaw = atan2f(sqrtf(ax * ax + ay * ay), az) * 180.0f / (float)M_PI;

    // Low-pass exponential smoothing for stable inclinometer angle estimation
    state.orientation.tiltX = 0.80f * state.orientation.tiltX + 0.20f * txRaw;
    state.orientation.tiltY = 0.80f * state.orientation.tiltY + 0.20f * tyRaw;
    state.orientation.tiltZ = 0.80f * state.orientation.tiltZ + 0.20f * tzRaw;

    state.orientation.pitch = state.orientation.tiltX;
    state.orientation.roll = state.orientation.tiltY;
    state.orientation.totalDeflection = sqrtf(state.orientation.pitch * state.orientation.pitch + 
                                              state.orientation.roll * state.orientation.roll);

    // Motion Detection & Double-Tap Navigation
    float accelMag = sqrtf(ax * ax + ay * ay + az * az);
    float gyroMag = sqrtf(gx * gx + gy * gy + gz * gz);

    uint32_t now = millis();
    static uint32_t motionHoldUntil = 0;
    if (fabsf(accelMag - 1.0f) > 0.15f || gyroMag > 20.0f) {
        motionHoldUntil = now + 1000;
        state.motion.motionDetected = true;
    } else {
        if (now >= motionHoldUntil) {
            state.motion.motionDetected = false;
        }
    }

    // Step 2: Periodic Structural FFT Computation (every 80 ms)
    static uint32_t livePeakHoldUntil = 0;
    static uint32_t lastSerialPrint = 0;

    if (_fftTimer.isReady() && _fft.isReady()) {
        float peakFreq = 0.0f;
        float peakMag = 0.0f;
        float bandEnergy = 0.0f;

        if (state.mode == MODE_CALIBRATING) {
            // Impulse detection (Tap-Test)
            if (fabsf(accelMag - 1.0f) > 0.25f || gyroMag > 35.0f) {
                state.structure.tapDetected = true;
            }

            _fft.compute(peakFreq, peakMag, bandEnergy, 12.5f, 3.0f);
            state.structure.livePeakFreqHz = peakFreq;
            state.structure.peakMagnitude = peakMag;

            // Only lock resonant peak if an actual physical tap excitation occurred
            if (state.structure.tapDetected && peakMag > 0.25f && peakFreq >= 2.0f && peakFreq <= 45.0f) {
                state.structure.resonantFreqHz = peakFreq;
            }
        } else if (state.mode == MODE_MONITORING) {
            // Continuous structural monitoring mode
            _fft.compute(peakFreq, peakMag, bandEnergy, state.structure.resonantFreqHz, 2.5f);

            // Noise gate: only register vibration if motion is detected or magnitude exceeds 0.25g
            if (!state.motion.motionDetected && peakMag < 0.25f) {
                peakFreq = 0.0f;
            }

            if (peakFreq > 0.0f) {
                state.structure.livePeakFreqHz = peakFreq;
                state.structure.peakMagnitude = peakMag;
                livePeakHoldUntil = now + 600; // Hold live frequency on screen for 0.6s
            } else {
                if (now >= livePeakHoldUntil) {
                    state.structure.livePeakFreqHz = 0.0f;
                    state.structure.peakMagnitude = 0.0f;
                }
            }

            state.structure.resonantBandEnergy = bandEnergy;

            // Serial debug output when motion/vibration is active
            if (state.structure.livePeakFreqHz > 0.0f && now - lastSerialPrint >= 150) {
                lastSerialPrint = now;
                Serial.print("[FFT LIVE] Peak: ");
                Serial.print(state.structure.livePeakFreqHz, 1);
                Serial.print(" Hz | Mag: ");
                Serial.print(state.structure.peakMagnitude, 2);
                Serial.print(" | Res Locked: ");
                Serial.print(state.structure.resonantFreqHz, 1);
                Serial.print(" Hz | Res E: ");
                Serial.println(state.structure.resonantBandEnergy, 2);
            }

            // Check if vibration energy is concentrated near resonant frequency
            bool freqNearResonance = (state.structure.livePeakFreqHz > 0.0f && 
                                     fabsf(state.structure.livePeakFreqHz - state.structure.resonantFreqHz) <= 2.5f);
            bool highVibrationEnergy = (bandEnergy > 2.5f || peakMag > 1.8f);

            if (freqNearResonance && highVibrationEnergy) {
                if (state.structure.sustainedCycles < 10) {
                    state.structure.sustainedCycles++;
                }
            } else {
                if (state.structure.sustainedCycles > 0) {
                    state.structure.sustainedCycles--;
                }
            }

            // Hazard latch: 5 consecutive cycles (~400 ms) of sustained resonance excitation
            if (state.structure.sustainedCycles >= 5) {
                state.structure.resonanceHazard = true;
            } else if (state.structure.sustainedCycles == 0) {
                state.structure.resonanceHazard = false;
            }
        }
    }

    return true;
}
