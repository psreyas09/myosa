/*
 * Sky-Mast - Handheld Smart Environmental & Orientation Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Bluetooth Low Energy (BLE) Controller for MYOSA Android App
 * File: ble.h
 */

#ifndef BLE_H
#define BLE_H

#include "utils.h"

class BLEController {
public:
    BLEController();
    bool begin(const char* deviceName = "MYOSA_1");
    void update(const SystemState &state);
    bool isConnected();

private:
    bool _initialized;
    bool _connected;
    NonBlockingTimer _timer;
};

#endif // BLE_H
