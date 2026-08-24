/*
 * Sky-Mast - Handheld Smart Environmental & Orientation Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Utilities & Data Definitions
 * File: utils.cpp
 */

#include "utils.h"

NonBlockingTimer::NonBlockingTimer(uint32_t intervalMs)
    : _interval(intervalMs), _previousMillis(0) {}

void NonBlockingTimer::setInterval(uint32_t intervalMs) {
    _interval = intervalMs;
}

bool NonBlockingTimer::isReady() {
    uint32_t currentMillis = millis();
    if (currentMillis - _previousMillis >= _interval) {
        _previousMillis = currentMillis;
        return true;
    }
    return false;
}

void NonBlockingTimer::reset() {
    _previousMillis = millis();
}

float calculatePitch(float accelX, float accelY, float accelZ) {
    // Calculate pitch angle in degrees using accelerometer components
    if (accelY == 0.0f && accelZ == 0.0f) return 0.0f;
    return atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0f / M_PI;
}

float calculateRoll(float accelX, float accelY, float accelZ) {
    // Calculate roll angle in degrees using accelerometer components
    if (accelZ == 0.0f) return 0.0f;
    return atan2(accelY, accelZ) * 180.0f / M_PI;
}

void formatFloat(float value, char* buffer, size_t bufferLen, uint8_t decimals) {
    dtostrf(value, 0, decimals, buffer);
}
