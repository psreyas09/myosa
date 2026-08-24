/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Barometric Pressure & Bernoulli Fluid Dynamics Wind Engine (BMP180)
 * File: barometer.h
 */

#ifndef BAROMETER_H
#define BAROMETER_H

#include <BarometricPressure.h>
#include "utils.h"

class BarometerController {
public:
    BarometerController();
    bool begin();
    bool update(SystemState &state);
    bool isConnected();

private:
    BarometricPressure _baro;
    NonBlockingTimer _timer; // 150 ms sampling rate
    bool _initialized;
    uint32_t _lastReadTime;
    float _baselinePressurePa;
    bool _baselineInitialized;
    uint8_t _galeCount;
};

#endif // BAROMETER_H
