/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Inertial Measurement Unit (MPU6050) & Structural Vibration Engine
 * File: imu.h
 */

#ifndef IMU_H
#define IMU_H

#include <AccelAndGyro.h>
#include "utils.h"
#include "fft_analyzer.h"

class IMUController {
public:
    IMUController();
    bool begin();
    bool update(SystemState &state);
    bool isConnected();
    void resetFFT();

private:
    AccelAndGyro _ag;
    StructuralFFTAnalyzer _fft;
    NonBlockingTimer _sampleTimer; // 10 ms (100 Hz) IMU sampling timer
    NonBlockingTimer _fftTimer;    // 80 ms FFT evaluation timer
    bool _initialized;
};

#endif // IMU_H
