/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Barometric Pressure & Bernoulli Fluid Dynamics Wind Engine (BMP180)
 * File: barometer.cpp
 */

#include "barometer.h"

#define AIR_DENSITY_RHO 1.225f // Standard dry air density at sea level (kg/m^3)

BarometerController::BarometerController()
    : _timer(150),
      _initialized(false),
      _lastReadTime(0),
      _baselinePressurePa(0.0f),
      _baselineInitialized(false),
      _galeCount(0) {
}

bool BarometerController::begin() {
    _initialized = _baro.begin();
    if (!_initialized) {
        _initialized = _baro.ping();
    }
    _baselinePressurePa = 0.0f;
    _baselineInitialized = false;
    _galeCount = 0;
    return _initialized;
}

bool BarometerController::isConnected() {
    return _baro.ping();
}

bool BarometerController::update(SystemState &state) {
    if (!_timer.isReady()) {
        return false;
    }

    bool connected = _baro.ping();
    state.sensorStatus.baroOk = connected;

    if (!connected) {
        state.rapidAltWarning = false;
        state.wind.windSpeedMs = 0.0f;
        state.wind.windSpeedKmh = 0.0f;
        state.wind.dynamicPressurePa = 0.0f;
        state.wind.highWindWarning = false;
        return false;
    }

    // Read barometric pressure in hPa (mbar) and calculate actual Pascals (1 hPa = 100 Pa)
    float hpa = _baro.getPressureBar(false);
    if (hpa <= 300.0f || hpa >= 1200.0f) {
        // Fallback if getPressureBar reading is invalid
        float rawPa = (float)_baro.getPressure();
        if (rawPa > 30000.0f && rawPa < 120000.0f) {
            hpa = rawPa / 100.0f;
        } else {
            hpa = 1013.25f;
        }
    }
    float pascal = hpa * 100.0f; // Exact pressure in Pascals (e.g. 101325.0 Pa)

    // Read ambient temperature in °C
    float tempC = _baro.getTempC(false);

    // Read altitude in meters using standard sea level reference (1013.25 hPa)
    float altMeters = _baro.getAltitude(1013.25f, false);

    state.env.pressure = hpa;
    state.env.temperature = tempC;

    // Smooth altitude reading using exponential low-pass filter (alpha = 0.30)
    if (state.env.altitude == 0.0f) {
        state.env.altitude = altMeters;
    } else {
        state.env.altitude = 0.70f * state.env.altitude + 0.30f * altMeters;
    }

    // Step 1: Bernoulli Fluid Dynamics Wind Speed Estimation
    // Initialize baseline ambient atmospheric pressure on first valid read
    if (!_baselineInitialized || _baselinePressurePa < 30000.0f) {
        _baselinePressurePa = pascal;
        _baselineInitialized = true;
    }

    // Adaptive baseline tracker:
    // If pressure rises, adapt quickly; if pressure drops (potential wind suction), adapt slowly
    if (pascal > _baselinePressurePa) {
        _baselinePressurePa = 0.90f * _baselinePressurePa + 0.10f * pascal;
    } else {
        _baselinePressurePa = 0.995f * _baselinePressurePa + 0.005f * pascal;
    }

    // Dynamic Bernoulli pressure drop: delta P = P_baseline - P_instantaneous
    float deltaP = _baselinePressurePa - pascal;
    if (deltaP < 0.0f) deltaP = 0.0f;

    state.wind.dynamicPressurePa = deltaP;

    // Fluid dynamics conversion: v = sqrt( (2 * deltaP) / rho )
    // Noise threshold: 6.0 Pa (~0.06 hPa) to ignore sensor quantization noise
    float rawWindMs = 0.0f;
    if (deltaP > 6.0f) {
        rawWindMs = sqrtf((2.0f * (deltaP - 6.0f)) / AIR_DENSITY_RHO);
    }

    // Exponential smoothing for steady wind speed readout
    state.wind.windSpeedMs = 0.85f * state.wind.windSpeedMs + 0.15f * rawWindMs;
    state.wind.windSpeedKmh = state.wind.windSpeedMs * 3.6f;

    // High Wind / Gale Alert threshold: > 17.0 m/s (~61 km/h) with debounce
    if (state.wind.windSpeedMs > 17.0f) {
        if (_galeCount < 5) _galeCount++;
        if (_galeCount >= 5) {
            state.wind.highWindWarning = true;
        }
    } else {
        _galeCount = 0;
        state.wind.highWindWarning = false;
    }

    // Step 2: Rate of elevation change
    uint32_t now = millis();
    if (_lastReadTime > 0) {
        float dt = (now - _lastReadTime) / 1000.0f;
        if (dt > 0.05f) {
            float altDiff = state.env.altitude - state.env.prevAltitude;
            state.env.altitudeRate = fabsf(altDiff) / dt;

            // Rapid elevation alert if rate > 1.6 m/s
            if (state.motion.motionDetected && state.env.altitudeRate > 1.6f) {
                state.rapidAltWarning = true;
            } else {
                state.rapidAltWarning = false;
            }
        }
    }

    _lastReadTime = now;
    state.env.prevAltitude = state.env.altitude;

    return true;
}
