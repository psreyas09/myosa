/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Gesture Sensor Fallback & Safety Bypass
 * File: gesture.cpp
 */

#include "gesture.h"
#include <Wire.h>

GestureController::GestureController()
    : _initialized(false), _timer(100) {
}

bool GestureController::begin() {
    // Non-blocking diagnostic probe of I2C address 0x39
    Wire.beginTransmission(0x39);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
        _initialized = true;
        Serial.println("[DIAGNOSTIC] APDS9960 detected on I2C 0x39 (Optional / Standby).");
    } else {
        _initialized = false;
        Serial.println("[INFO] APDS9960 Gesture sensor bypassed. System operating in Autonomous Self-Timed Mode.");
    }
    return _initialized;
}

bool GestureController::isConnected() {
    return _initialized;
}

bool GestureController::update(SystemState &state) {
    state.sensorStatus.gestureOk = _initialized;
    // Bypassed - system operates completely autonomously without gesture input requirements
    return false;
}
