/*
 * Sky-Mast - Handheld Smart Environmental & Orientation Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Bluetooth Serial SPP Controller for MYOSA Mobile App
 * File: bt.h
 */

#ifndef BT_H
#define BT_H

#include "utils.h"

class BTController {
public:
    BTController();
    bool begin(const char* deviceName = "MYOSA_1");
    void update(const SystemState &state);
    bool isConnected();

private:
    bool _initialized;
    NonBlockingTimer _timer;
};

#endif // BT_H
