/*
 * Sky-Mast - Handheld Smart Environmental & Structural Health Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Gesture Sensor Fallback & Safety Bypass
 * File: gesture.h
 */

#ifndef GESTURE_H
#define GESTURE_H

#include "utils.h"

class GestureController {
public:
    GestureController();
    bool begin();
    bool update(SystemState &state);
    bool isConnected();

private:
    bool _initialized;
    NonBlockingTimer _timer;
};

#endif // GESTURE_H
