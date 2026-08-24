/*
 * Sky-Mast - Handheld Smart Environmental & Orientation Monitor
 * MYOSA ESP32 Platform
 * 
 * Module: Bluetooth Low Energy (BLE) Controller for MYOSA Android App
 * File: ble.cpp
 */

#include "ble.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Arduino.h>

#define NUM_SERVICES          6
#define MAX_CHARACTERISTICS   5

static bool g_bleConnected = false;
static BLEServer *g_pServer = nullptr;
static BLECharacteristic *g_pCharacteristics[NUM_SERVICES][MAX_CHARACTERISTICS];

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        g_bleConnected = true;
        Serial.println("MYOSA BLE Device Connected!");
    }

    void onDisconnect(BLEServer* pServer) {
        g_bleConnected = false;
        Serial.println("MYOSA BLE Device Disconnected. Restarting Advertising...");
        pServer->getAdvertising()->start();
    }
};

BLEController::BLEController()
    : _initialized(false), _connected(false), _timer(500) {
}

bool BLEController::begin(const char* deviceName) {
    BLEDevice::init(deviceName);
    g_pServer = BLEDevice::createServer();
    g_pServer->setCallbacks(new MyServerCallbacks());

    for (int i = 0; i < NUM_SERVICES; i++) {
        char serviceUUID[37];
        int charIterator = (i == 0) ? 4 : (i == 1) ? 2 : (i == 5) ? 1 : 3;
        
        sprintf(serviceUUID, "4fafc201-1fb5-459e-8fcc-c5c9c33191b%d", i);
        BLEService *pService = g_pServer->createService(serviceUUID);

        for (int j = 0; j < charIterator; j++) {
            char characteristicUUID[37];
            sprintf(characteristicUUID, "beb5483e-36e1-4688-b7f5-ea07361b2b%d%d", i, j);

            if (i == 5 && j == 0) {
                g_pCharacteristics[i][j] = pService->createCharacteristic(
                    characteristicUUID,
                    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
                );
                // Exact identification string required by MYOSA Android App
                g_pCharacteristics[i][j]->setValue("Hello from MYOSA");
            } else {
                g_pCharacteristics[i][j] = pService->createCharacteristic(
                    characteristicUUID,
                    BLECharacteristic::PROPERTY_NOTIFY
                );
                g_pCharacteristics[i][j]->addDescriptor(new BLE2902());
            }
        }
        pService->start();
    }

    g_pServer->getAdvertising()->start();

    Serial.print("MYOSA BLE Advertising Started with Name: ");
    Serial.println(deviceName);
    _initialized = true;
    return true;
}

bool BLEController::isConnected() {
    return g_bleConnected;
}

void BLEController::update(const SystemState &state) {
    if (!_initialized || !_timer.isReady()) {
        return;
    }

    _connected = g_bleConnected;

    if (g_bleConnected) {
        char payload[40];

        // Service 0 (IMU Motion):
        // Char 0: Accel X, Y, Z
        if (state.sensorStatus.imuOk && g_pCharacteristics[0][0]) {
            sprintf(payload, "%0.2f, %0.2f, %0.2f", state.motion.accelX, state.motion.accelY, state.motion.accelZ);
            g_pCharacteristics[0][0]->setValue(payload);
            g_pCharacteristics[0][0]->notify();
        }

        // Char 1: Gyro X, Y, Z
        if (state.sensorStatus.imuOk && g_pCharacteristics[0][1]) {
            sprintf(payload, "%0.2f, %0.2f, %0.2f", state.motion.gyroX, state.motion.gyroY, state.motion.gyroZ);
            g_pCharacteristics[0][1]->setValue(payload);
            g_pCharacteristics[0][1]->notify();
        }

        // Char 2: Tilt X, Y, Z
        if (state.sensorStatus.imuOk && g_pCharacteristics[0][2]) {
            sprintf(payload, "%0.2f, %0.2f, %0.2f", state.orientation.tiltX, state.orientation.tiltY, state.orientation.tiltZ);
            g_pCharacteristics[0][2]->setValue(payload);
            g_pCharacteristics[0][2]->notify();
        }

        // Char 3: Temp C, Temp F
        if (state.sensorStatus.imuOk && g_pCharacteristics[0][3]) {
            sprintf(payload, "%0.2f, %0.2f", state.env.temperature, (state.env.temperature * 9.0f / 5.0f) + 32.0f);
            g_pCharacteristics[0][3]->setValue(payload);
            g_pCharacteristics[0][3]->notify();
        }

        // Service 2 (Barometer):
        // Char 0: Temp C, Temp F
        if (state.sensorStatus.baroOk && g_pCharacteristics[2][0]) {
            sprintf(payload, "%0.2f, %0.2f", state.env.temperature, (state.env.temperature * 9.0f / 5.0f) + 32.0f);
            g_pCharacteristics[2][0]->setValue(payload);
            g_pCharacteristics[2][0]->notify();
        }

        // Char 1: Pressure Pas, Pressure Hg, Pressure Bar
        if (state.sensorStatus.baroOk && g_pCharacteristics[2][1]) {
            float pas = state.env.pressure * 100.0f;
            float hg = pas * 0.00750062f;
            float bar = pas / 100000.0f;
            sprintf(payload, "%0.2f, %0.2f, %0.2f", pas, hg, bar);
            g_pCharacteristics[2][1]->setValue(payload);
            g_pCharacteristics[2][1]->notify();
        }

        // Char 2: Altitude
        if (state.sensorStatus.baroOk && g_pCharacteristics[2][2]) {
            sprintf(payload, "%0.2f", state.env.altitude);
            g_pCharacteristics[2][2]->setValue(payload);
            g_pCharacteristics[2][2]->notify();
        }
    }
}
